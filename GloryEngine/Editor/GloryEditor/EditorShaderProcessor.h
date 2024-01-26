#pragma once
#include <map>
#include <filesystem>
#include <shaderc/shaderc.hpp>
#include <ThreadedVar.h>
#include <ResourceMeta.h>
#include <JobManager.h>
#include <GloryEditor.h>

namespace Glory
{
	class ShaderSourceData;

namespace Editor
{
	struct AssetCallbackData;
	class EditorShaderData;

	class EditorShaderProcessor
	{
	public:
		static GLORY_EDITOR_API ShaderSourceData* GetShaderSource(UUID uuid);
		static GLORY_EDITOR_API EditorShaderData* GetEditorShader(UUID uuid);

		EditorShaderProcessor();
		virtual ~EditorShaderProcessor();

	private:
		static EditorShaderData* CompileAndCache(ShaderSourceData* pShaderSource, std::filesystem::path path);
		static EditorShaderData* LoadCache(ShaderSourceData* pShaderSource, std::filesystem::path path);

		void Start();
		void Stop();

		static void CompileForCurrentPlatform(EditorShaderData* pEditorShader, const std::string& path);
		static void ProcessReflection(EditorShaderData* pEditorShader);

		static void AssetCallback(const AssetCallbackData& callback);

		static bool LoadShaderSourceJob(UUID uuid);
		static bool CompileShaderJob(UUID uuid);

	private:
		friend class EditorApplication;
		static ThreadedUMap<UUID, ShaderSourceData*> m_pLoadedShaderSources;
		static ThreadedUMap<UUID, EditorShaderData*> m_pCompiledShaders;

		static Jobs::JobPool<bool, UUID>* m_pShaderJobsPool;

		UUID m_AssetRegisteredCallback;
		UUID m_AssetUpdatedCallback;
	};
}
}