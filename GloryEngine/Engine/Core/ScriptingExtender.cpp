#include "ScriptingExtender.h"
#include "ScriptExtensions.h"
#include "IScriptExtender.h"
#include "ScriptingModule.h"

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

		m_CurrentAppendingLanguage = "";
		m_pCurrentAppendingModule = nullptr;

		for (size_t i = 0; i < pEngine->m_pScriptingModules.size(); i++)
		{
			ScriptingModule* pScriptingModule = pEngine->m_pScriptingModules[i];
			const std::string& language = pScriptingModule->ScriptingLanguage();
			// Load libs
			for (size_t i = 0; i < m_Libs.size(); i++)
			{
				if (m_Libs[i].Language() != language) continue;
				pScriptingModule->AddLib(m_Libs[i]);
			}

			for (size_t i = 0; i < m_InternalCalls.size(); i++)
			{
				const InternalCall& internalCall = m_InternalCalls[i];
				if (internalCall.m_Language != language) continue;
				pScriptingModule->Bind(internalCall);
			}
		}
	}

	void ScriptingExtender::RegisterExtender(Module* pOwner, IScriptExtender* pExtender)
	{
		m_pExtenders.push_back(pExtender);
		m_pOwners[pExtender] = pOwner;
	}

	void ScriptingExtender::RegisterManagedExtender(Module* pOwner, IScriptExtender* pExtender)
	{
		m_pManagedExtenders.push_back(pExtender);
		m_pOwners[pExtender] = pOwner;
	}

	void ScriptingExtender::AddInternalLib(const std::string& name, void* data)
	{
		if (m_CurrentAppendingLanguage == "" || m_pCurrentAppendingModule == nullptr)
		{
			//m_pEngine->GetDebug().LogError("Cannot add internal libs outside of IScriptExtender::GetLibs call!");
			return;
		}

		const ModuleMetaData& metaData = m_pCurrentAppendingModule->GetMetaData();
		std::filesystem::path pathToLib = metaData.Path();
		pathToLib = pathToLib.parent_path().append("Scripting").append(m_CurrentAppendingLanguage);
		m_Libs.push_back(ScriptingLib(m_CurrentAppendingLanguage, name, pathToLib.string(), false, data));
	}

	size_t ScriptingExtender::InternalLibCount() const
	{
		return m_Libs.size();
	}

	const ScriptingLib& ScriptingExtender::GetInternalLib(size_t index) const
	{
		return m_Libs[index];
	}

	ScriptingExtender::ScriptingExtender() : m_pCurrentAppendingModule(nullptr)
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
		m_CurrentAppendingLanguage = pExtender->Language();
		m_pCurrentAppendingModule = m_pOwners[pExtender];

		pExtender->GetInternalCalls(m_InternalCalls);
		pExtender->GetLibs(this);
	}
}
