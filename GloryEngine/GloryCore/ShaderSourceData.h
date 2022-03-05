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
        ShaderSourceData(ShaderType shaderType, FileData* pCompiledSource);
        ShaderSourceData();
        virtual ~ShaderSourceData();

        size_t Size() const;
        const char* Data() const;
        const ShaderType& GetShaderType() const;
        FileData* GetCompiledShader() const;

        void SetCompiledShader(FileData* pCompiledShader);

    private:
		friend class ShaderSourceLoaderModule;
        std::vector<char> m_ProcessedSource;
        std::vector<char> m_OriginalSource;
        FileData* m_pPlatformCompiledShader;
        ShaderType m_ShaderType;
	};
}
