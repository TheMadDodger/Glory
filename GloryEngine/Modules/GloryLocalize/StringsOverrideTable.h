#pragma once
#include "StringTable.h"

namespace Glory
{
	class StringsOverrideTable : public StringTable
	{
	public:
		/** @brief Constructor */
		GLORY_API StringsOverrideTable();
		/** @override */
		GLORY_API StringsOverrideTable(UUID baseTableID, std::string&& language);
		/** @brief Destructor */
		GLORY_API virtual ~StringsOverrideTable();

		/** @brief Base table ID */
		GLORY_API UUID BaseTableID() const;
		/** @brief Language */
		GLORY_API const std::string& Language() const;

	private:
		/** @brief Get a vector containing other resources referenced by this resource */
		virtual void References(Engine*, std::vector<UUID>&) const override;

		virtual void Serialize(BinaryStream& container) const override;
		virtual void Deserialize(BinaryStream& container) override;

	private:
		UUID m_BaseTableID;
		std::string m_OverrideLanguage;
	};
}
