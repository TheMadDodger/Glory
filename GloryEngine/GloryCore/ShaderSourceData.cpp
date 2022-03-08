#include "ShaderSourceData.h"

namespace Glory
{
    ShaderSourceData::ShaderSourceData(ShaderType shaderType, FileData* pCompiledSource)
        : m_ProcessedSource(), m_OriginalSource(), m_ShaderType(shaderType), m_pPlatformCompiledShader(pCompiledSource)
    {}

    ShaderSourceData::ShaderSourceData() : m_ProcessedSource(), m_OriginalSource(), m_ShaderType(ShaderType::ST_Unknown), m_pPlatformCompiledShader(nullptr)
    {
    }

    ShaderSourceData::~ShaderSourceData()
    {
        m_ProcessedSource.clear();
        m_OriginalSource.clear();
        if (m_pPlatformCompiledShader) delete m_pPlatformCompiledShader;
        m_pPlatformCompiledShader = nullptr;
    }

    size_t ShaderSourceData::Size() const
    {
        return m_ProcessedSource.size();
    }

    const char* ShaderSourceData::Data() const
    {
        return m_ProcessedSource.data();
    }

    const ShaderType& ShaderSourceData::GetShaderType() const
    {
        return m_ShaderType;
    }

    FileData* ShaderSourceData::GetCompiledShader() const
    {
        return m_pPlatformCompiledShader;
    }

    void ShaderSourceData::SetCompiledShader(FileData* pShaderFile)
    {
        m_pPlatformCompiledShader = pShaderFile;
    }

    void ShaderSourceData::SetShaderResources(const spirv_cross::ShaderResources& resources)
    {
        m_Resources = resources;
    }

    const spirv_cross::ShaderResources& ShaderSourceData::GetResources() const
    {
        return m_Resources;
    }
}