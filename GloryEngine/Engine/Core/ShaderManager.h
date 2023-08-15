#pragma once
#include "UUID.h"
#include "ShaderData.h"
#include "FileData.h"
#include <functional>
#include <unordered_map>

namespace Glory
{
	class ShaderManager
	{
	public:
		static void OverrideCompiledShadersPathFunc(std::function<std::string()> func);
		static void OverrideMissingShaderHandlerFunc(std::function<void(UUID, std::function<void(FileData*)>)> func);
		static FileData* GetCompiledShaderFile(UUID uuid);

		static std::string GetCompiledShaderPath(UUID uuid);

	private:
		static void Cleanup();
		static void RunCallbacks();

	private:
		friend class GloryContext;
		ShaderManager();
		virtual ~ShaderManager();

	private:
		friend class Engine;
		std::function<std::string()> m_CompiledShadersPathFunc;
		std::function<void(UUID, std::function<void(FileData*)>)> m_MissingShaderHandlerFunc;
		std::unordered_map<UUID, FileData*> m_pLoadedShaderFiles;
	};
}
