#pragma once
#include "Resource.h"
#include "GraphicsEnums.h"
#include "MaterialPropertyInfo.h"

#include <BitSet.h>

namespace Glory
{
    class FileData;
    class MaterialData;
    class PipelineManager;

    /** @brief Pipeline data */
	class PipelineData : public Resource
	{
    public:
        /** @brief Constructor */
        PipelineData();
        /** @brief Destructor */
        virtual ~PipelineData();

        /** @brief Get the shading type for this pipeline */
        PipelineType Type() const;
        /** @brief Get the number of shaders attached to this pipeline */
        virtual size_t ShaderCount() const;
        /** @brief Get the ID of a shader attached to this pipeline
         * @param index Index of the shader
         */
        virtual UUID ShaderID(size_t index) const;

        /** @brief Get compiled shader attached to this pipeline
         * @param manager Shader manager to get shaders from
         * @param index Index of the shader to get
         */
        virtual const FileData* Shader(const PipelineManager& manager, size_t index) const;
        /** @brief Get the type of a shader attached to this pipeline
         * @param manager Shader manager to get shaders from
         * @param index Index of the shader to get the type from
         */
        virtual ShaderType GetShaderType(const PipelineManager& manager, size_t index) const;

        /** @brief Set the shading type for this pipeline
         * @param type @ref PipelineType to set
         */
        void SetPipelineType(PipelineType type);
        /** @brief Add a shader to the pipeline
         * @param shaderID ID of the shader to add
         */
        void AddShader(UUID shaderID);
        /** @brief Remove a shader from the pipeline
         * @param index Index of the shader to remove
         */
        void RemoveShaderAt(size_t index);

        /** @brief Add a property info to the pipeline */
        void AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, uint32_t flags = 0);
        /** @overload for resources */
        void AddResourceProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, TextureType textureType);
        /** @brief Get the number of properties for this pipeline */
        size_t PropertyInfoCount() const;
        /** @brief Get an @ref MaterialPropertyInfo by index */
        MaterialPropertyInfo* GetPropertyInfoAt(size_t index);
        /** @brief Clear all property infos */
        void ClearProperties();

        size_t ResourcePropertyCount() const;
        MaterialPropertyInfo* ResourcePropertyInfo(size_t resourceIndex);

        /** @brief Check if this pipeline is using a shader */
        bool HasShader(const UUID shaderID) const;

        /** @brief Remove all shaders from this pipeline */
        void RemoveAllShaders();

        /** @brief Serialize the pipeline into a binary stream */
        void Serialize(BinaryStream& container) const override;
        /** @brief Deserialize the pipeline from a binary stream */
        void Deserialize(BinaryStream& container) override;

        /** @brief Load properties into a material
         * @param pMaterial Destination material
         */
        void LoadIntoMaterial(MaterialData* pMaterial) const;
        
        /** @brief Check if this pipeline has texture parameters */
        bool UsesTextures() const;

        /** @brief Add a feature
         * @param feature The name of the feature
         * @param isOn Is the feature on by default
         */
        void AddFeature(std::string_view feature, bool isOn);

        size_t FeatureIndex(std::string_view feature) const;

        /** @brief Number of available features for this pipeline */
        size_t FeatureCount() const;

        /** @brief Get the name of a feature */
        std::string_view FeatureName(size_t index) const;

        /** @brief Check if a feature is enabled */
        bool FeatureEnabled(size_t index);

        /** @brief Enable a feature */
        void SetFeatureEnabled(size_t index, bool enabled);
        /** @brief Remove all features */
        void ClearFeatures();

    protected:
        PipelineType m_Type = PT_Phong;
        std::vector<UUID> m_Shaders;

        std::vector<MaterialPropertyInfo> m_PropertyInfos;
        std::vector<size_t> m_ResourcePropertyInfoIndices;
        std::unordered_map<uint32_t, size_t> m_HashToPropertyInfoIndex;
        std::vector<std::string> m_Features;
        Utils::BitSet m_FeaturesEnabled;

        size_t m_CurrentOffset = 0;
        size_t m_NumResources = 0;
	};
}
