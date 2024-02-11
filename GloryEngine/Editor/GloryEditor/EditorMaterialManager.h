#pragma once
#include <UUID.h>
#include <MaterialManager.h>

#include <map>

namespace Glory
{
	class MaterialData;
	class MaterialInstanceData;
	class Engine;

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
		/** @brief Unsibscribe asset events */
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

		/** @brief Add a shader to a material and update the materials properties
		 * @param materialID ID of the material
		 * @param shaderID ID of the shader
		 */
		void AddShaderToMaterial(UUID materialID, UUID shaderID);
		/** @brief Remove a shader from a material and update the materials properties
		 * @param materialID ID of the material
		 * @param index Index of the shader to remove
		 */
		void RemoveShaderFromMaterial(UUID materialID, size_t index);

		/** @brief Get a material or material instance by ID */
		virtual MaterialData* GetMaterial(UUID materialID) const override;

	private:
		/** @brief Handler for @ref AssetCallbackType::CT_AssetRegistered events */
		void AssetAddedCallback(const AssetCallbackData& callback);
		/** @brief Handler for @ref AssetCallbackType::CT_AssetUpdated events
		 * Currently does nothing
		 */
		void AssetUpdatedCallback(const AssetCallbackData& callback);

		/** @brief Read shaders into a material
		 * @param shaders Shaders YAML data
		 * @param pMaterial Material to add the shaders to
		 */
		void ReadShadersInto(Utils::NodeValueRef& shaders, MaterialData* pMaterial) const;
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

		/** @brief Handler for compiled shader events */
		void OnShaderCompiled(const UUID& uuid);

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
		std::map<UUID, MaterialData*> m_pMaterialDatas;
		std::map<UUID, MaterialInstanceData*> m_pMaterialInstanceDatas;
		std::map<UUID, std::vector<UUID>> m_WaitingMaterialInstances;

		Engine* m_pEngine;
		UUID m_AssetRegisteredCallback;
		UUID m_AssetUpdatedCallback;
		UUID m_ShaderCompiledCallback;
	};
}
}