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
		{
			container.Write(iter->first).Write(iter->second);
		}
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
}
