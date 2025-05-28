#include "StringTable.h"

#include <BinaryStream.h>
#include <Reflection.h>

namespace Glory
{
	StringTable::StringTable()
	{
		APPEND_TYPE(StringTable);
	}

	StringTable::~StringTable()
	{
	}

	void StringTable::AddString(std::string&& key, std::string&& value)
	{
		if (m_Strings.find(key) != m_Strings.end()) return;
		std::vector<std::string> tokens;
		Reflect::Tokenize(key, tokens, '.');
		m_Strings.emplace(std::move(key), std::move(value)).first->first;

		GroupData* group = &m_RootGroup;
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			std::string token = tokens[i];
			auto& iter = group->m_Subgroups.find(token);

			if (i == tokens.size() - 1)
			{
				group->m_Keys.emplace_back(std::move(token));
				break;
			}

			if (iter == group->m_Subgroups.end())
				iter = group->m_Subgroups.emplace(std::move(token), GroupData{}).first;

			group = &iter->second;
		}
	}

	void StringTable::UpdateString(const std::string& key, std::string&& value)
	{
		if (m_Strings.find(key) == m_Strings.end()) return;
		m_Strings[key] = std::move(value);
	}

	bool StringTable::HasKey(const std::string& key) const
	{
		return m_Strings.find(key) != m_Strings.end();
	}

	void StringTable::RemoveKey(const std::string& key)
	{
		if (m_Strings.find(key) == m_Strings.end()) return;
		m_Strings.erase(key);

		std::vector<std::string> tokens;
		Reflect::Tokenize(key, tokens, '.');

		GroupData* group = &m_RootGroup;
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			std::string token = tokens[i];
			auto& iter = group->m_Subgroups.find(token);

			if (i == tokens.size() - 1)
			{
				auto keyIter = std::find(group->m_Keys.begin(), group->m_Keys.end(), tokens.back());
				if (keyIter == group->m_Keys.end()) return;
				group->m_Keys.erase(keyIter);
				return;
			}

			if (iter == group->m_Subgroups.end()) return;
			group = &iter->second;
		}
	}

	std::vector<std::string>* StringTable::FindKeys(const std::string& path)
	{
		std::vector<std::string> tokens;
		Reflect::Tokenize(path, tokens, '.');

		GroupData* group = &m_RootGroup;
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			std::string token = tokens[i];
			auto& iter = group->m_Subgroups.find(token);
			if (iter == group->m_Subgroups.end()) return nullptr;
			group = &iter->second;
		}
		return &group->m_Keys;
	}

	void StringTable::FindSubgroups(const std::string& path, std::vector<std::string_view>& groups)
	{
		std::vector<std::string> tokens;
		Reflect::Tokenize(path, tokens, '.');

		GroupData* group = &m_RootGroup;
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			std::string token = tokens[i];
			auto& iter = group->m_Subgroups.find(token);
			if (iter == group->m_Subgroups.end()) return;
			group = &iter->second;
		}

		if (group->m_Subgroups.empty()) return;
		groups.reserve(group->m_Subgroups.size());
		for (auto& iter = group->m_Subgroups.begin(); iter != group->m_Subgroups.end(); ++iter)
			groups.push_back(iter->first);
	}

	void StringTable::FindKeysRecursively(const std::string& path, std::vector<std::string>& keys)
	{
		std::vector<std::string> tokens;
		Reflect::Tokenize(path, tokens, '.');
		GroupData* group = &m_RootGroup;
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			std::string token = tokens[i];
			auto& iter = group->m_Subgroups.find(token);
			if (iter == group->m_Subgroups.end()) return;
			group = &iter->second;
		}
		FindKeysRecursively("", group, keys);
	}

	StringTable::LookupResult StringTable::FindString(const std::string& key) const
	{
		auto& iter = m_Strings.find(key);
		if (iter == m_Strings.end()) return StringTable::LookupResult{ false, {} };
		return StringTable::LookupResult{ true, iter->second };
	}

	std::map<std::string, std::string>::const_iterator StringTable::Begin() const
	{
		return m_Strings.begin();
	}

	std::map<std::string, std::string>::const_iterator StringTable::End() const
	{
		return m_Strings.end();
	}

	void StringTable::References(Engine*, std::vector<UUID>&) const
	{
	}

	void StringTable::Serialize(BinaryStream& container) const
	{
		container.Write(m_Strings.size());
		for (auto iter = m_Strings.begin(); iter != m_Strings.end(); ++iter)
			container.Write(iter->first).Write(iter->second);
	}

	void StringTable::Deserialize(BinaryStream& container)
	{
		size_t numStrings = 0;
		container.Read(numStrings);
		for (size_t i = 0; i < numStrings; ++i)
		{
			std::string key, value;
			container.Read(key).Read(value);
			m_Strings.emplace(std::move(key), std::move(value));
		}
	}

	void StringTable::FindKeysRecursively(std::string basePath, GroupData* group, std::vector<std::string>& keys)
	{
		for (size_t i = 0; i < group->m_Keys.size(); ++i)
			keys.emplace_back(basePath.empty() ? group->m_Keys[i] : (basePath + "." + group->m_Keys[i]));

		for (auto& iter : group->m_Subgroups)
			FindKeysRecursively(basePath.empty() ? iter.first : basePath + "." + iter.first, &iter.second, keys);
	}
}
