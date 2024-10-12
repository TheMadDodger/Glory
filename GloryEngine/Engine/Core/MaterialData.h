#pragma once
#include "Resource.h"
#include "Texture.h"
#include "MaterialPropertyData.h"
#include "MaterialPropertyInfo.h"
#include "GraphicsEnums.h"
#include "AssetReference.h"

#include <vector>
#include <unordered_map>
#include <mutex>

namespace Glory
{
    class FileData;
    class ShaderManager;
    class MaterialManager;
    class PipelineData;
    class PipelineManager;

    class MaterialData : public Resource
    {
    public:
        MaterialData();
        virtual ~MaterialData();

        void AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, uint32_t flags = 0);
        void AddResourceProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, UUID resourceUUID, TextureType type, uint32_t flags = 0);
        void AddProperty(const MaterialPropertyInfo& other);

        void SetPipeline(PipelineData* pPipeline);
        void SetPipeline(UUID pipelineID);
        virtual PipelineData* GetPipeline(const MaterialManager&, const PipelineManager& pipelineManager) const;
        virtual UUID GetPipelineID(const MaterialManager&) const;

        [[nodiscard]]virtual size_t PropertyInfoCount(const MaterialManager& materialManager) const;
        virtual MaterialPropertyInfo* GetPropertyInfoAt(const MaterialManager& materialManager, size_t index);
        [[nodiscard]]virtual size_t GetCurrentBufferOffset(const MaterialManager& materialManager) const;
        virtual std::vector<char>& GetBufferReference(const MaterialManager& materialManager);
        virtual std::vector<char>& GetFinalBufferReference(MaterialManager& materialManager);
        virtual bool GetPropertyInfoIndex(const MaterialManager& materialManager, const std::string& name, size_t& index) const;
        virtual bool GetPropertyInfoIndex(const MaterialManager& materialManager, TextureType textureType, size_t texIndex, size_t& index) const;
        [[nodiscard]]size_t ResourceCount() const;
        virtual AssetReference<TextureData>* GetResourceUUIDPointer(MaterialManager& materialManager, size_t index);
        [[nodiscard]]virtual size_t GetResourcePropertyCount(MaterialManager& materialManager) const;
        virtual MaterialPropertyInfo* GetResourcePropertyInfo(MaterialManager& materialManager, size_t index);
        [[nodiscard]]virtual size_t GetPropertyIndexFromResourceIndex(MaterialManager& materialManager, size_t index) const;
        void ClearProperties();

        virtual size_t TextureCount(MaterialManager&, TextureType textureType) const;

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

    public: // Properties
        // Setters
        template<typename T>
        void Set(MaterialManager& materialManager, const std::string& name, const T& value)
        {
            size_t index;
            if (!GetPropertyInfoIndex(materialManager, name, index)) return;
            EnableProperty(index);
            m_PropertyInfos[index].Write<T>(GetPropertyBuffer(materialManager, index), value);
        }

        // Getters
        template<typename T>
        bool Get(MaterialManager& materialManager, const std::string& name, T& value)
        {
            size_t index;
            if (!GetPropertyInfoIndex(materialManager, name, index)) return false;
            return m_PropertyInfos[index].Read<T>(GetPropertyBuffer(materialManager, index), value);
        }

        virtual void SetTexture(MaterialManager& materialManager, const std::string& name, TextureData* value);
        virtual void SetTexture(MaterialManager& materialManager, const std::string& name, UUID uuid);
        virtual void SetTexture(MaterialManager& materialManager, TextureType textureType, size_t texIndex, UUID uuid);
        virtual bool GetTexture(MaterialManager& materialManager, const std::string& name, TextureData** value, AssetManager* pManager);

    protected:
        virtual void EnableProperty(size_t index);
        virtual std::vector<char>& GetPropertyBuffer(MaterialManager& pManager, size_t index);

    protected:
        friend class PipelineData;
        friend class MaterialInstanceData;

        UUID m_Pipeline;
        std::vector<MaterialPropertyInfo> m_PropertyInfos;
        std::vector<char> m_PropertyBuffer;

        std::vector<size_t> m_ResourcePropertyInfoIndices;
        std::vector<AssetReference<TextureData>> m_Resources;
        std::unordered_map<uint32_t, size_t> m_HashToPropertyInfoIndex;

        std::vector<std::vector<size_t>> m_TextureTypeIndices;

        size_t m_CurrentOffset;

        std::mutex m_PropertiesAccessMutex;
    };
}
