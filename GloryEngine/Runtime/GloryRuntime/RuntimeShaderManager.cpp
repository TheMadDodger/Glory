#include "RuntimeShaderManager.h"

namespace Glory
{
    RuntimeShaderManager::RuntimeShaderManager(Engine* pEngine): ShaderManager(pEngine)
    {
    }

    ShaderType RuntimeShaderManager::GetShaderType(UUID shaderID) const
    {
        return ShaderType();
    }

    FileData* RuntimeShaderManager::GetCompiledShaderFile(UUID shaderID) const
    {
        return nullptr;
    }
}
