#pragma once
#include "UUID.h"
#include "ShaderData.h"
#include "FileData.h"

#include <functional>
#include <unordered_map>

namespace Glory
{
	class Engine;

	class ShaderManager
	{
	public:
		virtual ~ShaderManager();

	public:
		void OverrideCompiledShadersPathFunc(std::function<std::string()> func);
		void OverrideMissingShaderHandlerFunc(std::function<void(UUID, std::function<void(FileData*)>)> func);
		FileData* GetCompiledShaderFile(UUID uuid);

		std::string GetCompiledShaderPath(UUID uuid);

	private:
		void Cleanup();
		void RunCallbacks();

	private:
		ShaderManager(Engine* pEngine);

	private:
		friend class Engine;
		Engine* m_pEngine;
		std::function<std::string()> m_CompiledShadersPathFunc;
		std::function<void(UUID, std::function<void(FileData*)>)> m_MissingShaderHandlerFunc;
		std::unordered_map<UUID, FileData*> m_pLoadedShaderFiles;
	};
}
