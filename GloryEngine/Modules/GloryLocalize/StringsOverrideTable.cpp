#include "StringsOverrideTable.h"

#include <BinaryStream.h>

namespace Glory
{
	StringsOverrideTable::StringsOverrideTable(): m_BaseTableID(0), m_OverrideLanguage("")
	{
		APPEND_TYPE(StringsOverrideTable);
	}

	StringsOverrideTable::StringsOverrideTable(UUID baseTableID, std::string&& language):
		m_BaseTableID(baseTableID), m_OverrideLanguage(std::move(language))
	{
		APPEND_TYPE(StringsOverrideTable);
	}

	StringsOverrideTable::~StringsOverrideTable()
	{
	}

	UUID StringsOverrideTable::BaseTableID() const
	{
		return m_BaseTableID;
	}

	void StringsOverrideTable::SetBaseTableID(UUID uuid)
	{
		m_BaseTableID = uuid;
	}

	const std::string& StringsOverrideTable::Language() const
	{
		return m_OverrideLanguage;
	}

	void StringsOverrideTable::SetLanguage(std::string&& language)
	{
		m_OverrideLanguage = std::move(language);
	}

	void StringsOverrideTable::References(Engine*, std::vector<UUID>& references) const
	{
		if (!m_BaseTableID) return;
		references.push_back(m_BaseTableID);
	}

	void StringsOverrideTable::Serialize(BinaryStream& container) const
	{
		container.Write(m_BaseTableID).Write(m_OverrideLanguage);
		StringTable::Serialize(container);
	}

	void StringsOverrideTable::Deserialize(BinaryStream& container)
	{
		container.Read(m_BaseTableID).Read(m_OverrideLanguage);
		StringTable::Deserialize(container);
	}
}
