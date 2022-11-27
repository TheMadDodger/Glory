#include "ScriptingModule.h"

namespace Glory
{
    ScriptingModule::ScriptingModule(const std::string& scriptingLanguage) : m_ScriptingLanguage(scriptingLanguage)
    {
    }

    ScriptingModule::~ScriptingModule()
    {
    }

    const std::string& ScriptingModule::ScriptingLanguage()
    {
        return m_ScriptingLanguage;
    }

    const std::type_info& ScriptingModule::GetModuleType()
    {
        return typeid(ScriptingModule);
    }
}
