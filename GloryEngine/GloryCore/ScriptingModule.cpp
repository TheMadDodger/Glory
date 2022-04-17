#include "ScriptingModule.h"

namespace Glory
{
    ScriptingModule::ScriptingModule()
    {
    }

    ScriptingModule::~ScriptingModule()
    {
    }

    const std::type_info& ScriptingModule::GetModuleType()
    {
        return typeid(ScriptingModule);
    }
}
