#pragma once
#include <UUID.h>
#include <GloryEditor.h>
#include <MaterialManager.h>

#include <map>

namespace Glory
{
	class MaterialData;
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
		 * @param node The YAML node to read from
		 * @param pMaterial Material to load the properties into
		 */
		GLORY_EDITOR_API void LoadIntoMaterial(Utils::NodeValueRef node, MaterialData* pMaterial, bool clearProperties=true) const;

		/** @brief Set a materials pipeline and update its properties
		 * @param materialID ID of the material
		 * @param pipelineID ID of the pipeline
		 */
		GLORY_EDITOR_API void SetMaterialPipeline(UUID materialID, UUID pipelineID);

		/** @brief Get a material or material instance by ID */
		GLORY_EDITOR_API virtual MaterialData* GetMaterial(UUID materialID) const override;

		GLORY_EDITOR_API virtual MaterialData* CreateRuntimeMaterial(UUID baseMaterial) override;

		GLORY_EDITOR_API virtual void DestroyRuntimeMaterials() override;

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
		/** @brief Write properties to YAML
		 * @param properties Properties YAML destination
		 * @param pMaterial Material to read the properties from
		 */
		void WritePropertiesTo(Utils::NodeValueRef& properties, MaterialData* pMaterial);

		/** @brief Update a material by loading the properties of its attached shaders and reload the YAML data if possible
		 * @param pMaterial Material to update
		 */
		void UpdateMaterial(MaterialData* pMaterial);

	private:
		std::vector<UUID> m_Materials;
		std::vector<UUID> m_RuntimeMaterials;

		Engine* m_pEngine;
		UUID m_AssetRegisteredCallback;
		UUID m_AssetUpdatedCallback;
		UUID m_PipelineUpdatedCallback;
	};
}
}