#pragma once
#include <string>

namespace Glory
{
	class ScriptBinding
	{
	public:
		ScriptBinding(const std::string& language, const std::string& scriptMethod, const void* pRedirectMethod);

		const std::string& Language() const;
		const std::string& ScriptMethod() const;
		const void* RedirectMethod() const;

	private:
		friend class ScriptingBinder;
		std::string m_Language;
		std::string m_ScriptMethod;
		const void* m_pRedirectMethod;
	};
}
