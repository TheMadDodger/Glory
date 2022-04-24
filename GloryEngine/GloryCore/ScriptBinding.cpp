#include "ScriptBinding.h"

namespace Glory
{
	ScriptBinding::ScriptBinding(const std::string& language, const std::string& scriptMethod, const void* pRedirectMethod)
		: m_Language(language), m_ScriptMethod(scriptMethod), m_pRedirectMethod(pRedirectMethod)
	{

	}

	const std::string& ScriptBinding::Language() const
	{
		return m_Language;
	}

	const std::string& ScriptBinding::ScriptMethod() const
	{
		return m_ScriptMethod;
	}

	const void* ScriptBinding::RedirectMethod() const
	{
		return m_pRedirectMethod;
	}
}
