#pragma once
#include <LocalizeModuleBase.h>
#include <Glory.h>

#include <map>
#include <string_view>

namespace Glory
{
	struct LocaleData
	{
		UUID m_BaseTableID;
		UUID m_OverrideTableID;
		std::string m_Language;
	};

	class StringsOverrideTable;

	/** @brief Localize module */
	class LocalizeModule : public LocalizeModuleBase
	{
	public:
		/** @brief Constructor */
		GLORY_API LocalizeModule();
		/** @brief Destructor */
		GLORY_API virtual ~LocalizeModule();

		/** @brief LocalizeModule type */
		GLORY_API virtual const std::type_info& GetModuleType() override;

		/** @brief Load a string table */
		GLORY_API void LoadStringTable(UUID tableID);
		GLORY_API void LoadStringOverrideTable(StringsOverrideTable* pTable);
		/** @brief Unload a string table */
		GLORY_API void UnloadStringTable(UUID tableID);
		GLORY_API void UnloadStringOverrideTable(UUID overrideTableID);
		/** @brief Clear all loaded tables */
		GLORY_API void Clear();

		/** @brief Find a string in the loaded tables
		 * @param tableName Name of the table to search in
		 * @param term Key of the string to find in the table
		 * @param out Output to write the result to if the term is found
		 * @returns true if the term was found, false otherwise
		 */
		GLORY_API bool FindString(const std::string_view tableName, const std::string_view term, std::string& out) override;

		GLORY_API void SetLanguages(std::string&& defaultLanguage, std::vector<std::string>&& supportedLanguages);
		GLORY_API void SetLocaleDatas(std::vector<LocaleData>&& localeDatas);
		GLORY_API void SetLanguage(std::string_view language);

		GLORY_MODULE_VERSION_H(0,1,0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

		virtual void LoadSettings(ModuleSettings& settings) override;
		virtual void LocalizeModule::OnProcessData() override;

		void RefreshText();

	private:
		struct LoadedTable
		{
			LoadedTable(std::string_view name) : m_Name(name) {}

			std::string_view m_Name;
			std::map<std::string_view, std::string_view> m_Strings;
		};

		std::string m_DefaultLanguage;
		std::string_view m_CurrentLanguage;
		std::vector<std::string> m_SupportedLanguages;

		std::vector<UUID> m_LoadedTableIDs;
		std::vector<LoadedTable> m_LoadedTables;
		std::vector<UUID> m_LoadedOverrideTableIDs;
		std::vector<LoadedTable> m_LoadedOverrideTables;
		std::vector<LocaleData> m_LocaleData;
	};
}
