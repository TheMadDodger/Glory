#pragma once
#include <LocalizeModuleBase.h>
#include <Glory.h>

#include <map>
#include <string_view>

namespace Glory
{
	class FSMData;
	class FSMState;
	struct FSMNode;

	class LocalizeModule : public LocalizeModuleBase
	{
	public:
		GLORY_API LocalizeModule();
		GLORY_API virtual ~LocalizeModule();

		GLORY_API virtual const std::type_info& GetModuleType() override;

		GLORY_API void LoadStringTable(UUID tableID);
		GLORY_API void UnloadStringTable(UUID tableID);
		GLORY_API void Clear();

		GLORY_API bool FindString(const std::string_view tableName, const std::string_view term, std::string& out) override;

		GLORY_MODULE_VERSION_H(0,1,0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		struct LoadedTable
		{
			LoadedTable(std::string_view name) : m_Name(name) {}

			std::string_view m_Name;
			std::map<std::string_view, std::string_view> m_Strings;
		};

		std::vector<UUID> m_LoadedTableIDs;
		std::vector<LoadedTable> m_LoadedTables;
	};
}
