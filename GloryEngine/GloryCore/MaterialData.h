#pragma once
#include "Resource.h"
#include "FileData.h"
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

        virtual size_t ShaderCount() const;
        virtual ShaderSourceData* GetShaderAt(size_t index) const;
        virtual const ShaderType& GetShaderTypeAt(size_t index) const;
        void RemoveShaderAt(size_t index);
        bool AddShader(ShaderSourceData* pShaderSourceData);

        void AddProperty(const std::string& displayName, const std::string& shaderName, size_t typeHash, size_t size, bool isResource, uint32_t flags = 0);
        void AddProperty(const std::string& displayName, const std::string& shaderName, size_t typeHash, UUID resourceUUID, uint32_t flags = 0);

        virtual size_t PropertyInfoCount() const;
        virtual MaterialPropertyInfo* GetPropertyInfoAt(size_t index);
        virtual size_t GetCurrentBufferOffset() const;
        virtual std::vector<char>& GetBufferReference();
        virtual std::vector<char>& GetFinalBufferReference();
        virtual bool GetPropertyInfoIndex(const std::string& name, size_t& index) const;
        size_t ResourceCount() const;
        virtual UUID* GetResourceUUIDPointer(size_t index);
        virtual size_t GetResourcePropertyCount() const;
        virtual MaterialPropertyInfo* GetResourcePropertyInfo(size_t index);
        virtual size_t GetPropertyIndexFromResourceIndex(size_t index) const;
        void ClearProperties();

    protected:
        friend class MaterialLoaderModule;
        friend class MaterialInstanceLoaderModule;
        std::vector<ShaderSourceData*> m_pShaderFiles;

        static std::hash<std::string> m_Hasher;

        std::vector<char> m_PropertyBuffer;
        std::vector<MaterialPropertyInfo> m_PropertyInfos;
        std::vector<size_t> m_ResourcePropertyInfoIndices;
        std::vector<UUID> m_Resources;
        std::unordered_map<size_t, size_t> m_HashToPropertyInfoIndex;

        size_t m_CurrentOffset;

        std::mutex m_PropertiesAccessMutex;
    };
}
