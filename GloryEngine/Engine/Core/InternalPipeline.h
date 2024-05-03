#pragma once
#include "PipelineData.h"
#include "GraphicsEnums.h"

namespace Glory
{
    class ShaderManager;
    class FileData;

    class InternalPipeline : public PipelineData
    {
    public:
        InternalPipeline(std::vector<FileData*>&& compiledShaders, std::vector<ShaderType>&& shaderTypes);

        virtual size_t ShaderCount() const override;
        virtual UUID ShaderID(size_t index) const override;
        virtual FileData* Shader(const ShaderManager& manager, size_t index) const override;
        virtual ShaderType GetShaderType(const ShaderManager& manager, size_t index) const override;

    private:
        std::vector<FileData*> m_pCompiledShaders;
        std::vector<ShaderType> m_ShaderTypes;
    };
}
