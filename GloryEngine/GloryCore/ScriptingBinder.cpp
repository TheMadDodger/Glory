#include "ScriptingBinder.h"
#include "ScriptBinding.h"

namespace Glory
{
	void ScriptingBinder::Initialize(Engine* pEngine)
	{
		std::vector<ScriptBinding> bindings;
		for (size_t i = 0; i < pEngine->m_pAllModules.size(); i++)
		{
			pEngine->m_pAllModules[i]->GetScriptBindings(bindings);
		}

		for (size_t i = 0; i < bindings.size(); i++)
		{
			const ScriptBinding& binding = bindings[i];
			for (size_t i = 0; i < pEngine->m_pScriptingModules.size(); i++)
			{
				ScriptingModule* pScriptingModule = pEngine->m_pScriptingModules[i];
				const std::string& language = pScriptingModule->ScriptingLanguage();
				if (binding.m_Language != language) continue;
				pScriptingModule->Bind(binding);
			}
		}
	}
}
