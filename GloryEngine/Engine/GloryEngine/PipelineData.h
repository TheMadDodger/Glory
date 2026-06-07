#pragma once
#include "Resource.h"
#include "GraphicsEnums.h"
#include "MaterialPropertyInfo.h"

#include <engine_visibility.h>

#include <BitSet.h>

#include <glm/vec4.hpp>

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
            BlendEnable = 24,
        };

    public:
        /** @brief Constructor */
        GLORY_ENGINE_API PipelineData();
        /** @overload */
        GLORY_ENGINE_API PipelineData(PipelineData&&) noexcept = default;
        GLORY_ENGINE_API PipelineData& operator=(PipelineData&&) noexcept = default;
        /** @brief Destructor */
        GLORY_ENGINE_API virtual ~PipelineData();

        /** @brief Get the shading type for this pipeline */
        GLORY_ENGINE_API PipelineType Type() const;
        /** @brief Get the number of shaders attached to this pipeline */
        GLORY_ENGINE_API virtual size_t ShaderCount() const;
        /** @brief Get the ID of a shader attached to this pipeline
         * @param index Index of the shader
         */
        GLORY_ENGINE_API virtual UUID ShaderID(size_t index) const;

        /** @brief Get compiled shader attached to this pipeline
         * @param manager Shader manager to get shaders from
         * @param index Index of the shader to get
         */
        GLORY_ENGINE_API virtual const FileData* Shader(const PipelineManager& manager, size_t index) const;
        /** @brief Get the type of a shader attached to this pipeline
         * @param manager Shader manager to get shaders from
         * @param index Index of the shader to get the type from
         */
        GLORY_ENGINE_API virtual ShaderType GetShaderType(const PipelineManager& manager, size_t index) const;

        /** @brief Set the shading type for this pipeline
         * @param type @ref PipelineType to set
         */
        GLORY_ENGINE_API void SetPipelineType(PipelineType type);
        /** @brief Add a shader to the pipeline
         * @param shaderID ID of the shader to add
         */
        GLORY_ENGINE_API void AddShader(UUID shaderID);
        /** @brief Remove a shader from the pipeline
         * @param index Index of the shader to remove
         */
        GLORY_ENGINE_API void RemoveShaderAt(size_t index);

        /** @brief Add a property info to the pipeline */
        GLORY_ENGINE_API void AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, uint32_t flags = 0);
        /** @overload for resources */
        GLORY_ENGINE_API void AddResourceProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, TextureType textureType);
        /** @brief Get the number of properties for this pipeline */
        GLORY_ENGINE_API size_t PropertyInfoCount() const;
        /** @brief Get an @ref MaterialPropertyInfo by index */
        GLORY_ENGINE_API MaterialPropertyInfo* GetPropertyInfoAt(size_t index);
        /** @brief Clear all property infos */
        GLORY_ENGINE_API void ClearProperties();

        /** @brief Add information on a uniform buffer to this pipeline
         * @param name Name of the buffer
         * @param shaderType Shader stage that uses this buffer
         *
         * Will update the ShaderTypeFlag bits if the buffer was already added.
         */
        GLORY_ENGINE_API void AddUniformBuffer(const std::string& name, ShaderType shaderType);
        /** @brief Add information on a storage buffer to this pipeline
         * @param name Name of the buffer
         * @param shaderType Shader stage that uses this buffer
         *
         * Will update the ShaderTypeFlag bits if the buffer was already added.
         */
        GLORY_ENGINE_API void AddStorageBuffer(const std::string& name, ShaderType shaderType);

        /** @brief Number of uniform buffers in this pipeline */
        GLORY_ENGINE_API size_t UniformBufferCount() const;
        /** @brief Number of storage buffers in this pipeline */
        GLORY_ENGINE_API size_t StorageBufferCount() const;

        /** @brief Get the info for a uniform buffer in this pipeline */
        GLORY_ENGINE_API const ShaderBufferInfo& UniformBuffer(size_t index) const;
        /** @brief Get the info for a storage buffer in this pipeline */
        GLORY_ENGINE_API const ShaderBufferInfo& StorageBuffer(size_t index) const;

        GLORY_ENGINE_API size_t ResourcePropertyCount() const;
        GLORY_ENGINE_API MaterialPropertyInfo* ResourcePropertyInfo(size_t resourceIndex);

        /** @brief Check if this pipeline is using a shader */
        GLORY_ENGINE_API bool HasShader(const UUID shaderID) const;

        /** @brief Remove all shaders from this pipeline */
        GLORY_ENGINE_API void RemoveAllShaders();

        /** @brief Serialize the pipeline into a binary stream */
        GLORY_ENGINE_API void Serialize(Utils::BinaryStream& container) const override;
        /** @brief Deserialize the pipeline from a binary stream */
        GLORY_ENGINE_API void Deserialize(Utils::BinaryStream& container) override;

        /** @brief Load properties into a material
         * @param pMaterial Destination material
         */
        GLORY_ENGINE_API void LoadIntoMaterial(MaterialData* pMaterial) const;
        
        /** @brief Check if this pipeline has texture parameters */
        GLORY_ENGINE_API bool UsesTextures() const;

        /** @brief Add a feature
         * @param feature The name of the feature
         * @param isOn Is the feature on by default
         */
        GLORY_ENGINE_API void AddFeature(std::string_view feature, bool isOn);

        /** @brief Add a define
         * @param define The name of the define
         */
        GLORY_ENGINE_API void AddDefine(std::string_view define);

        /** @brief Get the index of a feature
         * @param feature Name of the feature
         */
        GLORY_ENGINE_API size_t FeatureIndex(std::string_view feature) const;

        /** @brief Number of available features for this pipeline */
        GLORY_ENGINE_API size_t FeatureCount() const;
        /** @brief Number of defines for this pipeline */
        GLORY_ENGINE_API size_t DefineCount() const;

        /** @brief Get the name of a feature */
        GLORY_ENGINE_API std::string_view FeatureName(size_t index) const;
        /** @brief Get a define */
        GLORY_ENGINE_API std::string_view Define(size_t index) const;
        /** @brief Check if this pipeline has a define */
        GLORY_ENGINE_API bool HasDefine(std::string_view define) const;

        /** @brief Check if a feature is enabled */
        GLORY_ENGINE_API bool FeatureEnabled(size_t index) const;

        /** @brief Enable a feature */
        GLORY_ENGINE_API void SetFeatureEnabled(size_t index, bool enabled);
        /** @brief Remove all features */
        GLORY_ENGINE_API void ClearFeatures();
        /** @brief Remove all defines */
        GLORY_ENGINE_API void ClearDefines();

        /** @brief Get total size of properties buffer in bytes */
        GLORY_ENGINE_API size_t TotalPropertiesByteSize() const;

        /** @brief Are settings dirty */
        inline bool& SettingsDirty() { return m_SettingsDirty; }
        /** @overload */
        inline const bool& SettingsDirty() const { return m_SettingsDirty; }

        /** @brief Face to cull */
        inline CullFace& GetCullFace() { return m_CullFace; }
        /** @overload */
        inline const CullFace& GetCullFace() const { return m_CullFace; }

        /** @brief Primitive type to render */
        inline PrimitiveType& GetPrimitiveType() { return m_PrimitiveType; }
        /** @overload */
        inline const PrimitiveType& GetPrimitiveType() const { return m_PrimitiveType; }

        /** @brief Enable/disable depth test */
        GLORY_ENGINE_API void SetDepthTestEnabled(bool enable);
        /** @brief Is depth test enabled */
        GLORY_ENGINE_API const bool DepthTestEnabled() const;

        /** @brief Enable/disable depth write */
        GLORY_ENGINE_API void SetDepthWriteEnabled(bool enable);
        /** @brief Is depth write enabled */
        GLORY_ENGINE_API const bool DepthWriteEnabled() const;

        /** @brief Set color write mask */
        GLORY_ENGINE_API void SetColorWriteMask(bool r, bool g, bool b, bool a);
        /** @overload */
        GLORY_ENGINE_API void SetColorWriteMask(uint8_t mask);
        /** @brief Get color write mask */
        GLORY_ENGINE_API const void ColorWriteMask(bool& r, bool& g, bool& b, bool& a) const;
        /** @overload */
        GLORY_ENGINE_API const uint8_t ColorWriteMask() const;

        /** @brief Depth compare operator */
        inline CompareOp& GetDepthCompareOp() { return m_DepthCompare; }
        /** @overload */
        inline const CompareOp& GetDepthCompareOp() const { return m_DepthCompare; }

        /** @brief Enable/disable depth test */
        GLORY_ENGINE_API void SetStencilTestEnabled(bool enable);
        /** @brief Is depth test enabled */
        GLORY_ENGINE_API const bool StencilTestEnabled() const;

        /** @brief Set stencil compare mask */
        GLORY_ENGINE_API void SetStencilCompareMask(uint8_t mask);
        /** @brief Get stencil compare mask */
        GLORY_ENGINE_API const uint8_t StencilCompareMask() const;

        /** @brief Set stencil write mask */
        GLORY_ENGINE_API void SetStencilWriteMask(uint8_t mask);
        /** @brief Get stencil write mask */
        GLORY_ENGINE_API const uint8_t StencilWriteMask() const;

        /** @brief Set stencil reference */
        GLORY_ENGINE_API void SetStencilReference(uint8_t ref);
        /** @brief Get stencil reference */
        GLORY_ENGINE_API const uint8_t StencilReference() const;

        /** @brief Stencil compare operator */
        inline CompareOp& GetStencilCompareOp() { return m_StencilCompareOp; }
        /** @overload */
        inline const CompareOp& GetStencilCompareOp() const { return m_StencilCompareOp; }

        /** @brief Stencil fail func */
        inline Func& GetStencilFailOp() { return m_StencilFailOp; }
        /** @overload */
        inline const Func& GetStencilFailOp() const { return m_StencilFailOp; }

        /** @brief Stencil depth fail func */
        inline Func& GetStencilDepthFailOp() { return m_StencilDepthFailOp; }
        /** @overload */
        inline const Func& GetStencilDepthFailOp() const { return m_StencilDepthFailOp; }

        /** @brief Stencil pass func */
        inline Func& GetStencilPassOp() { return m_StencilPassOp; }
        /** @overload */
        inline const Func& GetStencilPassOp() const { return m_StencilPassOp; }

        /** @brief Enable/disable color blending */
        GLORY_ENGINE_API void SetBlendEnabled(bool enable);
        /** @brief Is color blending enabled */
        GLORY_ENGINE_API const bool BlendEnabled() const;

        inline BlendFactor& SrcColorBlendFactor() { return m_SrcColorBlendFactor; }
        inline const BlendFactor& SrcColorBlendFactor() const { return m_SrcColorBlendFactor; }

        inline BlendFactor& DstColorBlendFactor() { return m_DstColorBlendFactor; }
        inline const BlendFactor& DstColorBlendFactor() const { return m_DstColorBlendFactor; }

        inline BlendOp& ColorBlendOp() { return m_ColorBlendOp; }
        inline const BlendOp& ColorBlendOp() const { return m_ColorBlendOp; }

        inline BlendFactor& SrcAlphaBlendFactor() { return m_SrcAlphaBlendFactor; }
        inline const BlendFactor& SrcAlphaBlendFactor() const { return m_SrcAlphaBlendFactor; }

        inline BlendFactor& DstAlphaBlendFactor() { return m_DstAlphaBlendFactor; }
        inline const BlendFactor& DstAlphaBlendFactor() const { return m_DstAlphaBlendFactor; }

        inline BlendOp& AlphaBlendOp() { return m_AlphaBlendOp; }
        inline const BlendOp& AlphaBlendOp() const { return m_AlphaBlendOp; }

        inline glm::vec4& BlendConstants() { return m_BlendConstants; }
        inline const glm::vec4& BlendConstants() const { return m_BlendConstants; }

        /** @brief BitSet containing state of toggelable settings */
        inline const Utils::BitSet& SettingsTogglesBitSet() const { return m_SettingsToggles; }

    private:
        virtual void References(IEngine*, std::vector<UUID>&) const override {}

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
        std::vector<std::string> m_Defines;
        Utils::BitSet m_FeaturesEnabled;

        size_t m_CurrentOffset = 0;
        size_t m_NumResources = 0;

        bool m_SettingsDirty = false;

        /* Render settings */
        CullFace m_CullFace = CullFace::Back;
        PrimitiveType m_PrimitiveType = PrimitiveType::Triangles;

        /* Depth settings */
        CompareOp m_DepthCompare = CompareOp::OP_Less;

        /* Stencil settings */
        CompareOp m_StencilCompareOp = CompareOp::OP_Always;
        Func m_StencilFailOp = Func::OP_Zero;
        Func m_StencilDepthFailOp = Func::OP_Zero;
        Func m_StencilPassOp = Func::OP_Zero;

        /* Blend settings */
        BlendFactor m_SrcColorBlendFactor = BlendFactor::SrcAlpha;
        BlendFactor m_DstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
        BlendOp m_ColorBlendOp = BlendOp::Add;
        BlendFactor m_SrcAlphaBlendFactor = BlendFactor::One;
        BlendFactor m_DstAlphaBlendFactor = BlendFactor::OneMinusSrcAlpha;
        BlendOp m_AlphaBlendOp = BlendOp::Add;
        glm::vec4 m_BlendConstants = glm::vec4{};

        Utils::BitSet m_SettingsToggles{ 32, true };
	};
}
