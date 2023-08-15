#include "ShaderManager.h"
#include "Game.h"
#include "Engine.h"
#include "FileLoaderModule.h"
#include "Debug.h"
#include "GloryContext.h"

#include <filesystem>

#define SHADER_MANAGER GloryContext::GetContext()->GetShaderManager()

namespace Glory
{
	void ShaderManager::OverrideCompiledShadersPathFunc(std::function<std::string()> func)
	{
		SHADER_MANAGER->m_CompiledShadersPathFunc = func;
	}

	void ShaderManager::OverrideMissingShaderHandlerFunc(std::function<void(UUID, std::function<void(FileData*)>)> func)
	{
		SHADER_MANAGER->m_MissingShaderHandlerFunc = func;
	}

	FileData* ShaderManager::GetCompiledShaderFile(UUID uuid)
	{
		if (SHADER_MANAGER->m_pLoadedShaderFiles.find(uuid) != SHADER_MANAGER->m_pLoadedShaderFiles.end())
			return SHADER_MANAGER->m_pLoadedShaderFiles[uuid];

		std::filesystem::path cachePath = SHADER_MANAGER->m_CompiledShadersPathFunc();
		std::filesystem::path shaderPath = cachePath.append(std::to_string(uuid));

		if (!std::filesystem::exists(shaderPath)) return nullptr;

		FileImportSettings importSettings;
		importSettings.AddNullTerminateAtEnd = true;
		importSettings.Flags = std::ios::ate | std::ios::binary;
		importSettings.m_Extension = "";
		FileData* pShaderFile = (FileData*)Game::GetGame().GetEngine()->GetLoaderModule<FileData>()->Load(shaderPath.string(), importSettings);
		return pShaderFile;
	}

	std::string ShaderManager::GetCompiledShaderPath(UUID uuid)
	{
		std::filesystem::path cachePath = SHADER_MANAGER->m_CompiledShadersPathFunc();
		std::filesystem::path shaderPath = cachePath.append(std::to_string(uuid));
		return shaderPath.string();
	}

	void ShaderManager::Cleanup()
	{
		for (auto it = SHADER_MANAGER->m_pLoadedShaderFiles.begin(); it != SHADER_MANAGER->m_pLoadedShaderFiles.end(); it++)
		{
			delete it->second;
		}
		SHADER_MANAGER->m_pLoadedShaderFiles.clear();
	}

	void ShaderManager::RunCallbacks()
	{
	}

	ShaderManager::ShaderManager() : m_CompiledShadersPathFunc{ []() { return "./Shaders/"; } },
		m_MissingShaderHandlerFunc{ [](UUID uuid, std::function<void(FileData*)>) { Debug::LogFatalError("Missing shader! Please verify your game files!"); } },
		m_pLoadedShaderFiles{}
	{}

	ShaderManager::~ShaderManager()
	{
	}
}
