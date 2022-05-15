#include "ScriptingExtender.h"
#include "ScriptExtensions.h"

namespace Glory
{
	void ScriptingExtender::Initialize(Engine* pEngine)
	{
		size_t largestCount = m_pManagedExtenders.size() > m_pExtenders.size() ? m_pManagedExtenders.size() : m_pExtenders.size();

		for (size_t i = 0; i < largestCount; i++)
		{
			if (i < m_pExtenders.size()) LoadExtender(m_pExtenders[i]);
			if (i < m_pManagedExtenders.size()) LoadExtender(m_pManagedExtenders[i]);
		}

		for (size_t i = 0; i < pEngine->m_pScriptingModules.size(); i++)
		{
			ScriptingModule* pScriptingModule = pEngine->m_pScriptingModules[i];
			const std::string& language = pScriptingModule->ScriptingLanguage();
			// Load libs
			for (size_t i = 0; i < m_Libs.size(); i++)
			{
				if (m_Libs[i].Language() != language) continue;
				pScriptingModule->LoadLib(m_Libs[i]);
			}

			for (size_t i = 0; i < m_InternalCalls.size(); i++)
			{
				const InternalCall& internalCall = m_InternalCalls[i];
				if (internalCall.m_Language != language) continue;
				pScriptingModule->Bind(internalCall);
			}
			pScriptingModule->InitializeScripting();
		}
	}

	void ScriptingExtender::RegisterExtender(IScriptExtender* pExtender)
	{
		m_pExtenders.push_back(pExtender);
	}

	ScriptingExtender::ScriptingExtender()
	{
	}

	ScriptingExtender::~ScriptingExtender()
	{
		for (size_t i = 0; i < m_pManagedExtenders.size(); i++)
		{
			delete m_pManagedExtenders[i];
		}

		m_pManagedExtenders.clear();
		m_pExtenders.clear();
	}

	void ScriptingExtender::LoadExtender(IScriptExtender* pExtender)
	{
		pExtender->GetInternalCalls(m_InternalCalls);
		pExtender->GetLibs(m_Libs);
	}
}
