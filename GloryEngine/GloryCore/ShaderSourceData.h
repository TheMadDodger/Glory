#pragma once
#include "FileData.h"
#include "GraphicsEnums.h"
#include <string>
#include <vector>
#include <spirv_cross.hpp>

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
        void SetShaderResources(const spirv_cross::ShaderResources& resources);

        const spirv_cross::ShaderResources& GetResources() const;

    private:
		friend class ShaderSourceLoaderModule;
        std::vector<char> m_ProcessedSource;
        std::vector<char> m_OriginalSource;
        FileData* m_pPlatformCompiledShader;
        ShaderType m_ShaderType;
        spirv_cross::ShaderResources m_Resources;
	};
}
