#pragma once
#include <map>
#include <filesystem>
#include <shaderc/shaderc.hpp>
#include <ThreadedVar.h>
#include <ResourceMeta.h>
#include <JobManager.h>
#include <GloryEditor.h>
#include <ShaderManager.h>

namespace Glory
{
	class ShaderSourceData;

namespace Editor
{
	struct AssetCallbackData;
	class EditorShaderData;
	class EditorApplication;

	template<typename Arg>
	struct Dispatcher;

	struct ShaderCompiledEvent
	{
		const UUID ShaderID;
	};

	class EditorShaderProcessor : public ShaderManager
	{
	public:
		GLORY_EDITOR_API EditorShaderProcessor(EditorApplication* pApplication);
		GLORY_EDITOR_API virtual ~EditorShaderProcessor();

		GLORY_EDITOR_API static ShaderSourceData* GetShaderSource(UUID uuid);
		GLORY_EDITOR_API static bool GetShaderSource(UUID uuid, std::function<void(ShaderSourceData*)> callback);
		GLORY_EDITOR_API static EditorShaderData* GetEditorShader(UUID uuid);

		using ShaderCompiledDispatcher = Dispatcher<ShaderCompiledEvent>;
		GLORY_EDITOR_API static ShaderCompiledDispatcher& ShaderCompiledEventDispatcher();

		GLORY_EDITOR_API virtual ShaderType GetShaderType(UUID shaderID) const override;
		GLORY_EDITOR_API virtual FileData* GetCompiledShaderFile(UUID shaderID) const override;

		GLORY_EDITOR_API static TextureType ShaderNameToTextureType(const std::string_view name);

	private:
		static void CompileAndCache(ShaderSourceData* pShaderSource, std::filesystem::path path);
		static void LoadCache(UUID shaderID, std::filesystem::path path);

		void Start();
		void Stop();
		void RunCallbacks();

		static void CompileForCurrentPlatform(EditorShaderData* pEditorShader, const std::string& path);
		static void ProcessReflection(EditorShaderData* pEditorShader);

		static void AssetCallback(const AssetCallbackData& callback);
		static void AssetUpdatedCallback(const AssetCallbackData& callback);

		static bool LoadShaderSourceJob(UUID uuid);
		static bool CompileShaderJob(UUID uuid);

		static std::filesystem::path GetShaderSourceCachePath(UUID uuid);
		static std::filesystem::path GetCompiledShaderCachePath(UUID uuid);

	private:
		friend class EditorApplication;
		static ThreadedUMap<UUID, ShaderSourceData*> m_pLoadedShaderSources;
		static ThreadedUMap<UUID, EditorShaderData*> m_pCompiledShaders;
		static ThreadedVector<UUID> m_QueuedShaders;
		static ThreadedVector<UUID> m_FinishedShaders;

		static Jobs::JobPool<bool, UUID>* m_pShaderJobsPool;

		UUID m_AssetRegisteredCallback;
		UUID m_AssetUpdatedCallback;
	};
}
}