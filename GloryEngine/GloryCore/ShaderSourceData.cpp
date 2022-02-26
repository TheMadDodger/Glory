#include "ShaderSourceData.h"

namespace Glory
{
    ShaderSourceData::ShaderSourceData() : m_ProcessedSource(), m_OriginalSource(), m_ShaderType(ShaderType::ST_Unknown)
    {
    }

    ShaderSourceData::~ShaderSourceData()
    {
        m_ProcessedSource.clear();
        m_OriginalSource.clear();
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
}