#pragma once
#include "Resource.h"
#include "FileData.h"
#include <vector>
#include <unordered_map>
#include "GraphicsEnums.h"
#include "Texture.h"
#include "MaterialPropertyData.h"
#include <mutex>

namespace Glory
{
    class MaterialData : public Resource
    {
    public:
        MaterialData();
        MaterialData(const std::vector<FileData*>& shaderFiles, const std::vector<ShaderType>& shaderTypes);
        virtual ~MaterialData();

        virtual size_t ShaderCount() const;
        virtual FileData* GetShaderAt(size_t index) const;
        virtual const ShaderType& GetShaderTypeAt(size_t index) const;
        void RemoveShaderAt(size_t index);
         
        void AddProperty(const MaterialPropertyData& prop);
         
        size_t PropertyCount() const;
        MaterialPropertyData* GetPropertyAt(size_t index);
        MaterialPropertyData CopyPropertyAt(size_t index);
         
        virtual void CopyProperties(std::vector<MaterialPropertyData>& destination);
        virtual void PasteProperties(const std::vector<MaterialPropertyData>& destination);
        bool GetPropertyIndex(const std::string& name, size_t& index) const;

    protected:
        friend class MaterialLoaderModule;
        std::vector<FileData*> m_pShaderFiles;
        std::vector<ShaderType> m_ShaderTypes;

        static std::hash<std::string> m_Hasher;
        std::vector<MaterialPropertyData> m_Properties;
        std::unordered_map<size_t, size_t> m_HashToPropertyIndex;

        std::mutex m_PropertiesAccessMutex;
    };
}
