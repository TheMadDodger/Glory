#pragma once
#include <UUID.h>
#include <MaterialManager.h>

#include <map>

namespace Glory
{
	class MaterialData;
	class MaterialInstanceData;
	class Engine;
	class PipelineData;

namespace Utils
{
	struct YAMLFileRef;
	struct NodeValueRef;
}

namespace Editor
{
	struct AssetCallbackData;

	/** @brief Material manager for the editor */
	class EditorMaterialManager : public MaterialManager
	{
	public:
		/** @brief Constructor */
		EditorMaterialManager(Engine* pEngine);
		/** @brief Destructor */
		virtual ~EditorMaterialManager();

		/** @brief Initialize the manager by subscribing to asset events */
		void Initialize();
		/** @brief Unsubscribe asset events */
		void Cleanup();

		/** @brief Load editor property data from YAML into material
		 * @param file The YAML file to read from
		 * @param pMaterial Material to load the properties into
		 */
		void LoadIntoMaterial(Utils::YAMLFileRef& file, MaterialData* pMaterial) const;

		/** @overload Load editor property data from YAML into material instance
		 * @param file The YAML file to read from
		 * @param pMaterial Material instance to load the properties into
		 */
		void LoadIntoMaterial(Utils::YAMLFileRef& file, MaterialInstanceData*& pMaterial) const;

		/** @brief Set a materials pipeline and update its properties
		 * @param materialID ID of the material
		 * @param pipelineID ID of the pipeline
		 */
		void SetMaterialPipeline(UUID materialID, UUID pipelineID);

		/** @brief Set a material instance base material and update its properties
		 * @param materialInstanceID ID of the material instance
		 * @param baseMaterialID ID of the material to use as base
		 */
		void SetMaterialInstanceBaseMaterial(UUID materialInstanceID, UUID baseMaterialID);

		/** @brief Get a material or material instance by ID */
		virtual MaterialData* GetMaterial(UUID materialID) const override;

	private:
		/** @brief Handler for @ref AssetCallbackType::CT_AssetRegistered events */
		void AssetAddedCallback(const AssetCallbackData& callback);
		/** @brief Handler for @ref AssetCallbackType::CT_AssetUpdated events
		 * Currently does nothing
		 */
		void AssetUpdatedCallback(const AssetCallbackData& callback);

		/** @brief Callback for when a pipeline changes */
		void PipelineUpdateCallback(PipelineData* pPipeline);

		/** @brief Read properties into a material
		 * @param properties Properties YAML data
		 * @param pMaterial Material to read the properties to
		 * @param clearProperties Whether to clear the property data of the material before reading
		 */
		void ReadPropertiesInto(Utils::NodeValueRef& properties, MaterialData* pMaterial, bool clearProperties=true) const;
		/** @overload Read properties into a material instance
		 * @param properties Properties YAML data
		 * @param pMaterial Material instance to read the properties to
		 * @param clearProperties Whether to clear the property data of the material before reading
		 */
		void ReadPropertiesInto(Utils::NodeValueRef& properties, MaterialInstanceData* pMaterial, bool clearProperties=true) const;

		/** @brief Update a material by loading the properties of its attached shaders and reload the YAML data if possible
		 * @param pMaterial Material to update
		 */
		void UpdateMaterial(MaterialData* pMaterial);

		/** @brief Get a material instance by ID
		 * @param materialID ID of the material instance
		 */
		MaterialInstanceData* GetMaterialInstance(UUID materialID) const;

	private:
		friend class MaterialInstanceEditor;
		std::vector<UUID> m_Materials;
		std::vector<UUID> m_MaterialInstances;
		std::map<UUID, std::vector<UUID>> m_WaitingMaterialInstances;

		Engine* m_pEngine;
		UUID m_AssetRegisteredCallback;
		UUID m_AssetUpdatedCallback;
		UUID m_PipelineUpdatedCallback;
	};
}
}