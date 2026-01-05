#include "ShaderSourceData.h"
#include "BinaryStream.h"

namespace Glory
{
    ShaderSourceData::ShaderSourceData() : m_ProcessedSource(), m_OriginalSource(),
        m_ShaderType(ShaderType::ST_Unknown), m_TimeSinceLastWrite(0)
    {
        APPEND_TYPE(ShaderSourceData);
    }

    ShaderSourceData::ShaderSourceData(ShaderType shaderType, FileData* pCompiledSource)
        : m_ShaderType(shaderType), m_OriginalSource(),
        m_ProcessedSource(), m_TimeSinceLastWrite(0)
    {
        APPEND_TYPE(ShaderSourceData);
    }

    ShaderSourceData::ShaderSourceData(ShaderType shaderType, std::vector<char>&& source,
        std::vector<char>&& processed, std::vector<std::string>&& features,
        std::vector<std::filesystem::path>&& includes)
        : m_ShaderType(shaderType), m_OriginalSource(std::move(source)),
        m_ProcessedSource(std::move(processed)), m_Features(std::move(features)),
        m_Includes(std::move(includes)), m_IncludesLastWriteTimes(m_Includes.size(), 0ull),
        m_TimeSinceLastWrite(0)
    {
        APPEND_TYPE(ShaderSourceData);
    }

    ShaderSourceData::~ShaderSourceData()
    {
        m_ProcessedSource.clear();
        m_OriginalSource.clear();
        m_Includes.clear();
        m_IncludesLastWriteTimes.clear();
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

    void ShaderSourceData::Serialize(BinaryStream& container) const
    {
        container.Write(m_ShaderType);
    }

    void ShaderSourceData::Deserialize(BinaryStream& container)
    {
        container.Read(m_ShaderType);
    }

    uint64_t& ShaderSourceData::TimeSinceLastWrite()
    {
        return m_TimeSinceLastWrite;
    }

    const uint64_t& ShaderSourceData::TimeSinceLastWrite() const
    {
        return m_TimeSinceLastWrite;
    }

    size_t ShaderSourceData::IncludeCount() const
    {
        return m_Includes.size();
    }

    const std::filesystem::path& ShaderSourceData::IncludePath(size_t includeIndex) const
    {
        return m_Includes[includeIndex];
    }

    uint64_t& ShaderSourceData::TimeSinceLastWrite(size_t includeIndex)
    {
        return m_IncludesLastWriteTimes[includeIndex];
    }

    const uint64_t& ShaderSourceData::TimeSinceLastWrite(size_t includeIndex) const
    {
        return m_IncludesLastWriteTimes[includeIndex];
    }

    size_t ShaderSourceData::FeatureCount() const
    {
        return m_Features.size();
    }

    std::string_view ShaderSourceData::Feature(size_t index) const
    {
        return m_Features[index];
    }

    bool ShaderSourceData::IsOutdated(uint64_t cacheWriteTime) const
    {
        if (cacheWriteTime < m_TimeSinceLastWrite) return true;
        for (auto includeWriteTime : m_IncludesLastWriteTimes)
        {
            if (cacheWriteTime >= includeWriteTime) continue;
            return true;
        }
        return false;
    }
}