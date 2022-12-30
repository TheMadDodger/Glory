#pragma once
#include <ModuleMetaData.h>
#include <Versioning.h>

namespace Glory::EditorLauncher
{
	class ModuleManager
	{
	public:
		static void LoadModules(const std::filesystem::path& editorPath);
		static void Clear();

		static void GetModulesOfType(const ModuleType& moduleType, std::vector<ModuleMetaData*>& result);

	private:
		static void AddModuleMeta(const ModuleMetaData& metaData);

	private:
		ModuleManager();
		virtual ~ModuleManager();

	private:
		static std::vector<ModuleMetaData> m_InstalledModules;
		static std::map<ModuleType, std::vector<size_t>> m_ModulesByTypes;
	};
}
