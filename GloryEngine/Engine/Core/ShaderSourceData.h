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
        ShaderSourceData(ShaderType shaderType, std::vector<char>&& source, std::vector<char>&& processed);
        virtual ~ShaderSourceData();

        size_t Size() const;
        const char* Data() const;
        const ShaderType& GetShaderType() const;
        FileData* GetCompiledShader() const;

        void SetCompiledShader(FileData* pCompiledShader);

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

        uint64_t& TimeSinceLastWrite();
        const uint64_t& TimeSinceLastWrite() const;

    private:
        ShaderType m_ShaderType;
        FileData* m_pPlatformCompiledShader;
        std::vector<char> m_OriginalSource;
        std::vector<char> m_ProcessedSource;
        uint64_t m_TimeSinceLastWrite;
	};
}
