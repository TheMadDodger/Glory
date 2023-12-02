#include "ShaderSourceData.h"

namespace Glory
{
    ShaderSourceData::ShaderSourceData() : m_ProcessedSource(), m_OriginalSource(), m_ShaderType(ShaderType::ST_Unknown), m_pPlatformCompiledShader(nullptr)
    {
    }

    ShaderSourceData::ShaderSourceData(ShaderType shaderType, FileData* pCompiledSource)
        : m_ShaderType(shaderType), m_pPlatformCompiledShader(pCompiledSource), m_OriginalSource(), m_ProcessedSource()
    {}

    ShaderSourceData::ShaderSourceData(ShaderType shaderType, std::vector<char>&& source, std::vector<char>&& processed)
        : m_ShaderType(shaderType), m_pPlatformCompiledShader(nullptr), m_OriginalSource(std::move(source)), m_ProcessedSource(std::move(processed))
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
}