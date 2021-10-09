#pragma once
#include "Resource.h"
#include "FileData.h"
#include <vector>
#include <unordered_map>
#include "GraphicsEnums.h"
#include "Texture.h"

namespace Glory
{
    class MaterialData : public Resource
    {
    public:
        MaterialData(const std::vector<FileData*>& shaderFiles, const std::vector<ShaderType>& shaderTypes);
        virtual ~MaterialData();

        size_t ShaderCount() const;
        FileData* GetShaderAt(size_t index) const;
        const ShaderType& GetShaderTypeAt(size_t index) const;
        void SetTexture(ImageData* pTexture);
        ImageData* GetTexture();

    private:
        std::vector<FileData*> m_pShaderFiles;
        std::vector<ShaderType> m_ShaderTypes;
        ImageData* m_pTexture;
    };
}
