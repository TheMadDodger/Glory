#pragma once
#include "Resource.h"
#include "Texture.h"
#include "MaterialPropertyInfo.h"
#include "GraphicsEnums.h"
#include "AssetReference.h"

#include <vector>
#include <unordered_map>
#include <mutex>

namespace Glory
{
    class FileData;
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
        virtual PipelineData* GetPipeline(const PipelineManager& pipelineManager) const;
        virtual UUID GetPipelineID() const;

        [[nodiscard]]virtual size_t PropertyInfoCount() const;
        virtual MaterialPropertyInfo* GetPropertyInfoAt(size_t index);
        [[nodiscard]]virtual size_t GetCurrentBufferOffset() const;
        std::vector<char>& GetBufferReference();
        bool GetPropertyInfoIndex(const std::string& name, size_t& index) const;
        bool GetPropertyInfoIndex(TextureType textureType, size_t texIndex, size_t& index) const;
        [[nodiscard]]size_t ResourceCount() const;
        AssetReference<TextureData>* GetResourceUUIDPointer(size_t index);
        [[nodiscard]]virtual size_t GetResourcePropertyCount() const;
        MaterialPropertyInfo* GetResourcePropertyInfo(size_t index);
        [[nodiscard]]virtual size_t GetPropertyIndexFromResourceIndex(size_t index) const;
        void ClearProperties();

        size_t TextureCount(TextureType textureType) const;

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

        void References(Engine* pEngine, std::vector<UUID>& references) const override;

        void CopyProperties(void* dst);
        size_t PropertyDataSize();

        MaterialData* CreateCopy() const;

    public: // Properties
        // Setters
        template<typename T>
        void Set(const std::string& name, const T& value)
        {
            size_t index;
            if (!GetPropertyInfoIndex(name, index)) return;
            GetPropertyInfoAt(index)->Write<T>(m_PropertyBuffer, value);
        }

        // Getters
        template<typename T>
        bool Get(const std::string& name, T& value)
        {
            size_t index;
            if (!GetPropertyInfoIndex(name, index)) return false;
            return GetPropertyInfoAt(index)->Read<T>(m_PropertyBuffer, value);
        }

        GLORY_API void* Address(size_t index);

        GLORY_API void SetTexture(const std::string& name, TextureData* value);
        GLORY_API void SetTexture(const std::string& name, UUID uuid);
        GLORY_API void SetTexture(TextureType textureType, size_t texIndex, UUID uuid);
        GLORY_API bool GetTexture(const std::string& name, TextureData** value, AssetManager* pManager);

    protected:
        friend class PipelineData;

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
