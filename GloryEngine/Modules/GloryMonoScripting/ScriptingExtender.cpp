#include "ScriptingExtender.h"
#include "ScriptExtensions.h"
#include "IScriptExtender.h"
#include "GloryMonoScipting.h"

namespace Glory
{
	void ScriptingExtender::Initialize(GloryMonoScipting* pScripting)
	{
		pScripting->GetLibs(this);
		pScripting->GetInternalCalls(m_InternalCalls);

		for (size_t i = 0; i < m_pExtenders.size(); i++)
		{
			m_pExtenders[i]->GetInternalCalls(m_InternalCalls);
			m_pExtenders[i]->GetLibs(this);
		}

		// Load libs
		for (size_t i = 0; i < m_Libs.size(); i++)
		{
			pScripting->AddLib(m_Libs[i]);
		}

		for (size_t i = 0; i < m_InternalCalls.size(); i++)
		{
			const InternalCall& internalCall = m_InternalCalls[i];
			pScripting->Bind(internalCall);
		}
	}

	void ScriptingExtender::RegisterExtender(IScriptExtender* pExtender)
	{
		m_pExtenders.push_back(pExtender);
	}

	void ScriptingExtender::AddInternalLib(const std::string& location, const std::string& name, void* data)
	{
		std::filesystem::path pathToLib = location;
		m_Libs.push_back(ScriptingLib(name, pathToLib.string(), false, data));
	}

	size_t ScriptingExtender::InternalLibCount() const
	{
		return m_Libs.size();
	}

	const ScriptingLib& ScriptingExtender::GetInternalLib(size_t index) const
	{
		return m_Libs[index];
	}

	ScriptingExtender::ScriptingExtender()
	{
	}

	ScriptingExtender::~ScriptingExtender()
	{
		for (size_t i = 0; i < m_pExtenders.size(); i++)
		{
			delete m_pExtenders[i];
		}

		m_pExtenders.clear();
	}
}
