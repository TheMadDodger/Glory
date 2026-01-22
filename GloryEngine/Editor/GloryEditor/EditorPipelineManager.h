#pragma once
#include <PipelineManager.h>
#include <UUID.h>
#include <GraphicsEnums.h>
#include <ThreadedVar.h>
#include <NodeRef.h>
#include <GloryEditor.h>

#include <map>
#include <set>
#include <filesystem>

namespace Glory
{
	class Engine;
	class BinaryStream;
	class ShaderSourceData;

	namespace Jobs
	{
		template<typename ret, typename ...args>
		class JobPool;
	}
}

namespace Glory::Editor
{
	struct AssetCallbackData;
	class EditorShaderData;
	class EditorPipeline;

	template<typename Arg>
	struct Dispatcher;

	/** @brief Data for pipeline update events */
	struct PipelineUpdateEvent
	{
		PipelineData* pPipeline;
	};

	/** @brief Pipeline manager for the editor */
	class EditorPipelineManager : public PipelineManager
	{
	public:
		/** @brief Constructor */
		EditorPipelineManager(Engine* pEngine);
		/** @brief Destructor */
		virtual ~EditorPipelineManager();

		/** @brief Initialize the manager by subscribing to asset events */
		void Initialize();
		/** @brief Unsubscribe asset events */
		void Cleanup();

		/** @brief Add a shader to a pipeline and update the cached properties
		 * @param pipelineID ID of the pipeline
		 * @param shaderID ID of the shader
		 */
		void AddShaderToPipeline(UUID pipelineID, UUID shaderID);
		/** @brief Remove a shader from a pipeline and update the cached properties
		 * @param materialID ID of the pipeline
		 * @param index Index of the shader to remove
		 */
		void RemoveShaderFromPipeline(UUID pipelineID, size_t index);

		void SetPipelineFeatureEnabled(UUID pipelineID, std::string_view feature, bool enable);

		/** @brief Get a pipeline by ID
		 * @param pipelineID ID of the pipeline to get
		 */
		virtual PipelineData* GetPipelineData(UUID pipelineID) const override;

		virtual const std::vector<FileData>& GetPipelineCompiledShaders(UUID pipelineID) const override;
		virtual const std::vector<ShaderType>& GetPipelineShaderTypes(UUID pipelineID) const override;

		/** @brief Add a compiled shader, not allowed in the editor */
		virtual void AddShader(FileData* pShader) override;

		GLORY_EDITOR_API UUID FindPipeline(PipelineType type, bool useTextures) const;

		/** @brief Finds a pipeline by type and closest match with provided texture types
		 * @param type The type of the pipeline
		 * @param textureTypes The texture types that are used
		 */
		GLORY_EDITOR_API UUID FindPipeline(PipelineType type, const std::set<TextureType>& textureTypes) const;
		/** @overload Does the same as the method above but only matches texture types, which is slower */
		GLORY_EDITOR_API UUID FindPipeline(const std::set<TextureType>& textureTypes) const;

		using PipelineUpdateDispatcher = Dispatcher<PipelineUpdateEvent>;

		/** @brief Get pipeline updates event dispatcher */
		static PipelineUpdateDispatcher& PipelineUpdateEvents();

		/** @brief Get the @ref TextureType from a samplers name */
		static TextureType ShaderNameToTextureType(std::string_view name);

		static ShaderSourceData* GetShaderSource(UUID shaderID);

		void RunCallbacks();

		FileData* CompileShader(ShaderSourceData* pShaderSource);

	private:
		/** @brief Handler for @ref AssetCallbackType::CT_AssetRegistered events */
		void AssetAddedCallback(const AssetCallbackData& callback);
		/** @brief Handler for @ref AssetCallbackType::CT_AssetUpdated events
		 * Currently does nothing
		 */
		void AssetUpdatedCallback(const AssetCallbackData& callback);

		/** @brief Handler for compiled shader events */
		//void OnShaderCompiled(const UUID& uuid);

		/** @brief Update a pipeline by loading the properties of its attached shaders
		 * @param pPipeline Pipeline to update
		 */
		void UpdatePipeline(PipelineData* pPipeline, EditorPipeline* pEditorPipeline);

		/** @brief Load YAML data into a pipeline
		 * @param node YAML node to load from
		 * @param pPipeline pipeline to load the data into
		 */
		void LoadIntoPipeline(Utils::NodeValueRef node, PipelineData* pPipeline) const;

		/** @brief Check whether compiled shader cache is outdated
		 * @param cachePath Path to shader cache file
		 * @param shaderSource Original shader data
		 *
		 * Compares the last write date of the cache to that of the shader
		 */
		bool IsCacheOutdated(const std::filesystem::path& cachePath, ShaderSourceData* shaderSource);

		/** @brief Compile a pipeline for use in the editor
		 * @param pPipeline Pipeline to compile
		 */
		EditorPipeline* CompilePipelineForEditor(PipelineData* pPipeline);

		/** @brief Compile the platform shader for use in the editor
		 * @param pEditorPipeline Pipeline to compile
		 */
		void CompileForEditorPlatform(EditorPipeline* pEditorPipeline);

		/** @brief Get path to compiled pipeline cache file */
		std::filesystem::path GetCompiledPipelineCachePath(UUID uuid);

		/** @brief Load the original and processed shader source */
		ShaderSourceData* LoadOriginalShader(UUID uuid);
		/** @brief Run shaderc reflection to store shader properties
		 * @param pEditorShader Data to process and store reflecdtion on
		 */
		void ProcessReflection(EditorShaderData* pEditorShader);

		bool IsBusy();
		void WaitIdle();

		void QueueCompileJob(UUID pipelineID);

		bool CompilePipelineJob(UUID pipelineID);

		void DeletePipelineCache(UUID pipelineID);

	private:
		std::vector<UUID> m_Pipelines;
		std::vector<std::vector<FileData>> m_CompiledShaders;
		std::vector<std::vector<ShaderType>> m_ShaderTypes;

		UUID m_AssetRegisteredCallback;
		UUID m_AssetUpdatedCallback;

		Jobs::JobPool<bool, UUID>* m_pPipelineJobsPool;
		static ThreadedVector<UUID> m_QueuedPipelines;
		static ThreadedVector<EditorPipeline*> m_FinishedPipelines;
		static std::mutex m_WaitMutex;
		static std::condition_variable m_WaitCondition;
		static ThreadedUMap<UUID, ShaderSourceData*> m_pLoadedShaderSources;
		static std::unordered_map<UUID, size_t> m_ShaderVersions;
		static std::vector<ShaderSourceData*> m_pOutdatedShaders;
	};
}
