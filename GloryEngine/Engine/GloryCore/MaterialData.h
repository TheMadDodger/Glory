#pragma once
#include "Resource.h"
#include "ShaderSourceData.h"
#include "Texture.h"
#include "MaterialPropertyData.h"
#include "MaterialPropertyInfo.h"
#include "GraphicsEnums.h"
#include <vector>
#include <unordered_map>
#include <mutex>

namespace Glory
{
    class MaterialData : public Resource
    {
    public:
        MaterialData();
        MaterialData(const std::vector<ShaderSourceData*>& shaderFiles);
        virtual ~MaterialData();

        [[nodiscard]]virtual size_t ShaderCount() const;
        [[nodiscard]]virtual ShaderSourceData* GetShaderAt(size_t index) const;
        [[nodiscard]]virtual const ShaderType& GetShaderTypeAt(size_t index) const;
        void RemoveShaderAt(size_t index);
        bool AddShader(ShaderSourceData* pShaderSourceData);

        void AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, bool isResource, uint32_t flags = 0);
        void AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, UUID resourceUUID, uint32_t flags = 0);

        [[nodiscard]]virtual size_t PropertyInfoCount() const;
        virtual MaterialPropertyInfo* GetPropertyInfoAt(size_t index);
        [[nodiscard]]virtual size_t GetCurrentBufferOffset() const;
        virtual std::vector<char>& GetBufferReference();
        virtual std::vector<char>& GetFinalBufferReference();
        virtual bool GetPropertyInfoIndex(const std::string& name, size_t& index) const;
        [[nodiscard]]size_t ResourceCount() const;
        virtual UUID* GetResourceUUIDPointer(size_t index);
        [[nodiscard]]virtual size_t GetResourcePropertyCount() const;
        virtual MaterialPropertyInfo* GetResourcePropertyInfo(size_t index);
        [[nodiscard]]virtual size_t GetPropertyIndexFromResourceIndex(size_t index) const;
        void ClearProperties();

    public: // Properties
        // Setters
        template<typename T>
        void Set(const std::string& name, const T& value)
        {
            size_t index;
            if (!GetPropertyInfoIndex(name, index)) return;
            EnableProperty(index);
            m_PropertyInfos[index].Write<T>(GetPropertyBuffer(index), value);
        }

        // Getters
        template<typename T>
        bool Get(const std::string& name, T& value)
        {
            size_t index;
            if (!GetPropertyInfoIndex(name, index)) return false;
            return m_PropertyInfos[index].Read<T>(GetPropertyBuffer(index), value);
        }

        virtual void SetTexture(const std::string& name, ImageData* value);
        virtual bool GetTexture(const std::string& name, ImageData** value);

    protected:
        virtual void EnableProperty(size_t index);
        virtual std::vector<char>& GetPropertyBuffer(size_t index);

    protected:
        friend class MaterialLoaderModule;
        friend class MaterialInstanceLoaderModule;
        friend class MaterialInstanceData;
        std::vector<ShaderSourceData*> m_pShaderFiles;

        std::vector<char> m_PropertyBuffer;
        std::vector<MaterialPropertyInfo> m_PropertyInfos;
        std::vector<size_t> m_ResourcePropertyInfoIndices;
        std::vector<UUID> m_Resources;
        std::unordered_map<uint32_t, size_t> m_HashToPropertyInfoIndex;

        size_t m_CurrentOffset;

        std::mutex m_PropertiesAccessMutex;
    };
}
