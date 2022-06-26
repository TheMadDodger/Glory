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
}