#include "ShaderManager.h"
#include "Engine.h"
#include "FileLoaderModule.h"
#include "Debug.h"

#include <filesystem>

namespace Glory
{
	void ShaderManager::OverrideCompiledShadersPathFunc(std::function<std::string()> func)
	{
		m_CompiledShadersPathFunc = func;
	}

	void ShaderManager::OverrideMissingShaderHandlerFunc(std::function<void(UUID, std::function<void(FileData*)>)> func)
	{
		m_MissingShaderHandlerFunc = func;
	}

	FileData* ShaderManager::GetCompiledShaderFile(UUID uuid)
	{
		if (m_pLoadedShaderFiles.find(uuid) != m_pLoadedShaderFiles.end())
			return m_pLoadedShaderFiles[uuid];

		std::filesystem::path cachePath = m_CompiledShadersPathFunc();
		std::filesystem::path shaderPath = cachePath.append(std::to_string(uuid));

		if (!std::filesystem::exists(shaderPath)) return nullptr;

		FileImportSettings importSettings;
		importSettings.AddNullTerminateAtEnd = true;
		importSettings.Flags = std::ios::ate | std::ios::binary;
		importSettings.m_Extension = "";
		FileData* pShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(shaderPath.string(), importSettings);
		return pShaderFile;
	}

	std::string ShaderManager::GetCompiledShaderPath(UUID uuid)
	{
		std::filesystem::path cachePath = m_CompiledShadersPathFunc();
		std::filesystem::path shaderPath = cachePath.append(std::to_string(uuid));
		return shaderPath.string();
	}

	void ShaderManager::Cleanup()
	{
		for (auto it = m_pLoadedShaderFiles.begin(); it != m_pLoadedShaderFiles.end(); it++)
		{
			delete it->second;
		}
		m_pLoadedShaderFiles.clear();
	}

	void ShaderManager::RunCallbacks()
	{
	}

	ShaderManager::ShaderManager(Engine* pEngine) : m_pEngine(pEngine), m_CompiledShadersPathFunc{[]() { return "./Shaders/"; }},
		m_MissingShaderHandlerFunc{ [this](UUID uuid, std::function<void(FileData*)>) { m_pEngine->GetDebug().LogFatalError("Missing shader! Please verify your game files!"); }},
		m_pLoadedShaderFiles{}
	{}

	ShaderManager::~ShaderManager()
	{
	}
}
