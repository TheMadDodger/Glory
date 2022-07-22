#include "Module.h"
#include "GloryContext.h"

namespace Glory
{
    Module::Module() : m_pEngine(nullptr)
    {
    }

    Module::~Module()
    {
    }

    Engine* Module::GetEngine()
    {
        return m_pEngine;
    }

    bool Module::HasPriority()
    {
        return false;
    }

    const ModuleMetaData& Module::GetMetaData() const
    {
        return m_MetaData;
    }

    void Module::SetMetaData(const std::filesystem::path& path)
    {
        m_MetaData = ModuleMetaData(path);
        m_MetaData.Read();
    }
}