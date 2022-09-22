#include "pch.h"
#include "ModuleManager.h"
#include <filesystem>

namespace Glory::EditorLauncher
{
	std::vector<ModuleMetaData> ModuleManager::m_InstalledModules;
	std::map<ModuleType, std::vector<size_t>> ModuleManager::m_ModulesByTypes;

	void ModuleManager::LoadModules(const std::filesystem::path& editorPath)
	{
		Clear();

		std::filesystem::path installedModulesPath = editorPath;
		installedModulesPath.append("Modules");

		for (const auto& entry : std::filesystem::directory_iterator(installedModulesPath))
		{
			if (!entry.is_directory()) continue;
			std::filesystem::path metaFilePath = entry.path();
			metaFilePath.append("Module.yaml");
			if (!std::filesystem::exists(metaFilePath)) continue;

			ModuleMetaData moduleMeta(metaFilePath);
			moduleMeta.Read();
			AddModuleMeta(moduleMeta);
		}
	}

	void ModuleManager::Clear()
	{
		m_InstalledModules.clear();
		m_ModulesByTypes.clear();
	}

	void ModuleManager::GetModulesOfType(const ModuleType& moduleType, std::vector<ModuleMetaData*>& result)
	{
		if (m_ModulesByTypes.find(moduleType) == m_ModulesByTypes.end()) return;
		for (size_t i = 0; i < m_ModulesByTypes[moduleType].size(); i++)
		{
			size_t index = m_ModulesByTypes[moduleType][i];
			result.push_back(&m_InstalledModules[index]);
		}
	}

	void ModuleManager::AddModuleMeta(const ModuleMetaData& metaData)
	{
		size_t index = m_InstalledModules.size();
		m_InstalledModules.push_back(metaData);
		m_ModulesByTypes[metaData.Type()].push_back(index);
	}

	ModuleManager::ModuleManager()
	{
	}

	ModuleManager::~ModuleManager()
	{
	}
}
