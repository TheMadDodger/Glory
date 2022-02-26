#pragma once
#include "FileData.h"
#include <string>
#include <vector>
#include "GraphicsEnums.h"

namespace Glory
{
	class ShaderSourceData : public Resource
	{
    public:
        ShaderSourceData();
        virtual ~ShaderSourceData();

        size_t Size() const;
        const char* Data() const;
        const ShaderType& GetShaderType() const;

    private:
		friend class ShaderSourceLoaderModule;
        std::vector<char> m_ProcessedSource;
        std::vector<char> m_OriginalSource;
        ShaderType m_ShaderType;
	};
}
