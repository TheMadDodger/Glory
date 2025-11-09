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
        enum SettingBitsIndices : uint8_t
        {
            DepthTestEnable = 0,
            DepthWriteEnable = 1,
            ColorWriteRed = 2,
            ColorWriteGreen = 3,
            ColorWriteBlue = 4,
            ColorWriteAlpha = 5,
            StencilTestEnable = 6,
            StencilCompareMaskBegin = 7,
            StencilWriteMaskBegin = 15,
            StencilReferenceBegin = 23,
        };

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

        /** @brief Add information on a uniform buffer to this pipeline
         * @param name Name of the buffer
         * @param shaderType Shader stage that uses this buffer
         *
         * Will update the ShaderTypeFlag bits if the buffer was already added.
         */
        void AddUniformBuffer(const std::string& name, ShaderType shaderType);
        /** @brief Add information on a storage buffer to this pipeline
         * @param name Name of the buffer
         * @param shaderType Shader stage that uses this buffer
         *
         * Will update the ShaderTypeFlag bits if the buffer was already added.
         */
        void AddStorageBuffer(const std::string& name, ShaderType shaderType);

        /** @brief Number of uniform buffers in this pipeline */
        size_t UniformBufferCount() const;
        /** @brief Number of storage buffers in this pipeline */
        size_t StorageBufferCount() const;

        /** @brief Get the info for a uniform buffer in this pipeline */
        const ShaderBufferInfo& UniformBuffer(size_t index) const;
        /** @brief Get the info for a storage buffer in this pipeline */
        const ShaderBufferInfo& StorageBuffer(size_t index) const;

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

        /** @brief Get the index of a feature
         * @param feature Name of the feature
         */
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

        /** @brief Get total size of properties buffer in bytes */
        size_t TotalPropertiesByteSize() const;

        /** @brief Are settings dirty */
        bool& SettingsDirty() { return m_SettingsDirty; }
        /** @overload */
        const bool& SettingsDirty() const { return m_SettingsDirty; }

        /** @brief Face to cull */
        CullFace& GetCullFace() { return m_CullFace; }
        /** @overload */
        const CullFace& GetCullFace() const { return m_CullFace; }

        /** @brief Primitive type to render */
        PrimitiveType& GetPrimitiveType() { return m_PrimitiveType; }
        /** @overload */
        const PrimitiveType& GetPrimitiveType() const { return m_PrimitiveType; }

        /** @brief Enable/disable depth test */
        void SetDepthTestEnabled(bool enable);
        /** @brief Is depth test enabled */
        const bool DepthTestEnabled() const;

        /** @brief Enable/disable depth write */
        void SetDepthWriteEnabled(bool enable);
        /** @brief Is depth write enabled */
        const bool DepthWriteEnabled() const;

        /** @brief Set color write mask */
        void SetColorWriteMask(bool r, bool g, bool b, bool a);
        /** @overload */
        void SetColorWriteMask(uint8_t mask);
        /** @brief Get color write mask */
        const void ColorWriteMask(bool& r, bool& g, bool& b, bool& a) const;
        /** @overload */
        const uint8_t ColorWriteMask() const;

        /** @brief Depth compare operator */
        CompareOp& GetDepthCompareOp() { return m_DepthCompare; }
        /** @overload */
        const CompareOp& GetDepthCompareOp() const { return m_DepthCompare; }

        /** @brief Enable/disable depth test */
        void SetStencilTestEnabled(bool enable);
        /** @brief Is depth test enabled */
        const bool StencilTestEnabled() const;

        /** @brief Set stencil compare mask */
        void SetStencilCompareMask(uint8_t mask);
        /** @brief Get stencil compare mask */
        const uint8_t StencilCompareMask() const;

        /** @brief Set stencil write mask */
        void SetStencilWriteMask(uint8_t mask);
        /** @brief Get stencil write mask */
        const uint8_t StencilWriteMask() const;

        /** @brief Set stencil reference */
        void SetStencilReference(uint8_t ref);
        /** @brief Get stencil reference */
        const uint8_t StencilReference() const;

        /** @brief Stencil compare operator */
        CompareOp& GetStencilCompareOp() { return m_StencilCompareOp; }
        /** @overload */
        const CompareOp& GetStencilCompareOp() const { return m_StencilCompareOp; }

        /** @brief Stencil fail func */
        Func& GetStencilFailOp() { return m_StencilFailOp; }
        /** @overload */
        const Func& GetStencilFailOp() const { return m_StencilFailOp; }

        /** @brief Stencil depth fail func */
        Func& GetStencilDepthFailOp() { return m_StencilDepthFailOp; }
        /** @overload */
        const Func& GetStencilDepthFailOp() const { return m_StencilDepthFailOp; }

        /** @brief Stencil pass func */
        Func& GetStencilPassOp() { return m_StencilPassOp; }
        /** @overload */
        const Func& GetStencilPassOp() const { return m_StencilPassOp; }

        /** @brief BitSet containing state of toggelable settings */
        const Utils::BitSet& SettingsTogglesBitSet() const { return m_SettingsToggles; }

    private:
        virtual void References(Engine*, std::vector<UUID>&) const override {}

    protected:
        PipelineType m_Type = PT_Phong;
        std::vector<UUID> m_Shaders;

        size_t m_TotalPropertiesByteSize;

        std::vector<MaterialPropertyInfo> m_PropertyInfos;
        std::vector<size_t> m_ResourcePropertyInfoIndices;
        std::unordered_map<uint32_t, size_t> m_HashToPropertyInfoIndex;
        std::vector<ShaderBufferInfo> m_UniformBuffers;
        std::vector<ShaderBufferInfo> m_StorageBuffers;
        std::vector<std::string> m_Features;
        Utils::BitSet m_FeaturesEnabled;

        size_t m_CurrentOffset = 0;
        size_t m_NumResources = 0;

        bool m_SettingsDirty = false;
        CullFace m_CullFace = CullFace::Back;
        PrimitiveType m_PrimitiveType = PrimitiveType::Triangles;
        CompareOp m_DepthCompare = CompareOp::OP_Less;
        CompareOp m_StencilCompareOp = CompareOp::OP_Always;
        Func m_StencilFailOp = Func::OP_Zero;
        Func m_StencilDepthFailOp = Func::OP_Zero;
        Func m_StencilPassOp = Func::OP_Zero;
        Utils::BitSet m_SettingsToggles{ 32, true };
	};
}
