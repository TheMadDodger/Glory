#include "StringTable.h"

#include <BinaryStream.h>

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
		m_Strings.emplace(std::move(key), std::move(value));
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
		container.Write(numStrings);
		for (size_t i = 0; i < numStrings; ++i)
		{
			std::string key, value;
			container.Read(key).Read(value);
			m_Strings.emplace(std::move(key), std::move(value));
		}
	}
}
