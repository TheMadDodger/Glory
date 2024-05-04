#pragma once
#include <PipelineManager.h>
#include <UUID.h>

#include <map>

namespace Glory
{
	class Engine;
	class PipelineData;


	namespace Utils
	{
		struct YAMLFileRef;
	}
}

namespace Glory::Editor
{
	struct AssetCallbackData;

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

		virtual PipelineData* GetPipelineData(UUID pipelineID) const override;

	private:
		/** @brief Handler for @ref AssetCallbackType::CT_AssetRegistered events */
		void AssetAddedCallback(const AssetCallbackData& callback);
		/** @brief Handler for @ref AssetCallbackType::CT_AssetUpdated events
		 * Currently does nothing
		 */
		void AssetUpdatedCallback(const AssetCallbackData& callback);

		/** @brief Handler for compiled shader events */
		void OnShaderCompiled(const UUID& uuid);

		/** @brief Update a pipeline by loading the properties of its attached shaders
		 * @param pPipeline Pipeline to update
		 */
		void UpdatePipeline(PipelineData* pPipeline);

		void LoadIntoPipeline(Utils::YAMLFileRef& file, PipelineData* pPipeline) const;

	private:
		std::map<UUID, PipelineData*> m_pPipelineDatas;
		std::map<uint64_t, UUID> m_DefaultPipelinesMap;

		UUID m_AssetRegisteredCallback;
		UUID m_ShaderCompiledCallback;
	};
}
