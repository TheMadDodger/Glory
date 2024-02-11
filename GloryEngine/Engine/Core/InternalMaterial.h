#pragma once
#include "MaterialData.h"

namespace Glory
{
    class InternalMaterial : public MaterialData
    {
    public:
        InternalMaterial(std::vector<FileData*>&& compiledShaders, std::vector<ShaderType>&& shaderTypes);

        virtual size_t ShaderCount(const MaterialManager& materialManager) const override;
        virtual ShaderType GetShaderTypeAt(const MaterialManager& materialManager, ShaderManager& manager, size_t index) const override;
        virtual UUID GetShaderIDAt(const MaterialManager& materialManager, size_t index) const override;
        virtual FileData* GetShaderAt(const MaterialManager& materialManager, ShaderManager& manager, size_t index) const override;

    private:
        std::vector<FileData*> m_pCompiledShaders;
        std::vector<ShaderType> m_ShaderTypes;
    };
}
