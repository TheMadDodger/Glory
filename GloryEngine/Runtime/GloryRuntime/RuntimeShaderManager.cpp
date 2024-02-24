#include "RuntimeShaderManager.h"

#include <Engine.h>
#include <ShaderSourceData.h>

namespace Glory
{
    RuntimeShaderManager::RuntimeShaderManager(Engine* pEngine): ShaderManager(pEngine)
    {
    }

    ShaderType RuntimeShaderManager::GetShaderType(UUID shaderID) const
    {
        auto itor = m_pShaders.find(shaderID);
        if (itor == m_pShaders.end()) return ShaderType::ST_Unknown;
        return itor->second->GetShaderType();
    }

    FileData* RuntimeShaderManager::GetCompiledShaderFile(UUID shaderID) const
    {
        auto itor = m_pShaders.find(shaderID);
        if (itor == m_pShaders.end()) return nullptr;
        return itor->second->GetCompiledShader();
    }

    void RuntimeShaderManager::AddShader(ShaderSourceData* pShader)
    {
        m_pShaders.emplace(pShader->GetUUID(), pShader);
    }
}
