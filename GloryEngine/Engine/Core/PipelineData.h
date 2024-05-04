#pragma once
#include "Resource.h"
#include "GraphicsEnums.h"
#include "MaterialPropertyInfo.h"

namespace Glory
{
    class FileData;
    class ShaderManager;
    class MaterialData;

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

        virtual FileData* Shader(const ShaderManager& manager, size_t index) const;
        virtual ShaderType GetShaderType(const ShaderManager& manager, size_t index) const;

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
        void AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash);
        /** @brief Get the number of properties for this pipeline */
        size_t PropertyInfoCount() const;
        /** @brief Get an @ref MaterialPropertyInfo by index */
        MaterialPropertyInfo* GetPropertyInfoAt(size_t index);
        /** @brief Clear all property infos */
        void ClearProperties();

        /** @brief Check if this pipeline is using a shader */
        bool HasShader(const UUID shaderID) const;

        /** @brief Remove all shaders from this pipeline */
        void RemoveAllShaders();

        /** @brief Serialize the pipeline into a binary stream */
        void Serialize(BinaryStream& container) const override;
        /** @brief Deserialize the pipeline from a binary stream */
        void Deserialize(BinaryStream& container) override;

        void LoadIntoMaterial(MaterialData* pMaterial) const;

        bool UsesTextures() const;

    protected:
        PipelineType m_Type = PT_Phong;
        std::vector<UUID> m_Shaders;

        std::vector<MaterialPropertyInfo> m_PropertyInfos;
        std::vector<size_t> m_ResourcePropertyInfoIndices;
        std::unordered_map<uint32_t, size_t> m_HashToPropertyInfoIndex;

        size_t m_CurrentOffset = 0;
        size_t m_NumResources = 0;
	};
}
