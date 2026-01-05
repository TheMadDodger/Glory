#pragma once
#include "FileData.h"
#include "GraphicsEnums.h"
#include <string>
#include <vector>

namespace Glory
{
	class ShaderSourceData : public Resource
	{
    public:
        ShaderSourceData();
        ShaderSourceData(ShaderType shaderType, FileData* pCompiledSource);
        ShaderSourceData(ShaderType shaderType, std::vector<char>&& source,
            std::vector<char>&& processed, std::vector<std::string>&& features,
            std::vector<std::filesystem::path>&& includes);
        virtual ~ShaderSourceData();

        size_t Size() const;
        const char* Data() const;
        const ShaderType& GetShaderType() const;

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

        uint64_t& TimeSinceLastWrite();
        const uint64_t& TimeSinceLastWrite() const;

        size_t IncludeCount() const;
        const std::filesystem::path& IncludePath(size_t includeIndex) const;
        uint64_t& TimeSinceLastWrite(size_t includeIndex);
        const uint64_t& TimeSinceLastWrite(size_t includeIndex) const;

        size_t FeatureCount() const;
        std::string_view Feature(size_t index) const;

        bool IsOutdated(uint64_t cacheWriteTime) const;

    private:
        virtual void References(Engine*, std::vector<UUID>&) const override {}

    private:
        ShaderType m_ShaderType;
        std::vector<char> m_OriginalSource;
        std::vector<char> m_ProcessedSource;
        std::vector<std::string> m_Features;
        std::vector<std::filesystem::path> m_Includes;
        std::vector<uint64_t> m_IncludesLastWriteTimes;
        uint64_t m_TimeSinceLastWrite;
	};
}
