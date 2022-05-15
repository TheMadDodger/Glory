#include "ScriptExtensions.h"

namespace Glory
{
	ScriptExtension::ScriptExtension(const std::string& language) : m_Language(language)
	{
	}

	ScriptExtension::~ScriptExtension()
	{
	}

	const std::string& ScriptExtension::Language() const
	{
		return m_Language;
	}

	ScriptingLib::ScriptingLib(const std::string& language, const std::string& libraryName) : ScriptExtension(language), m_LibraryName(libraryName)
	{

	}

	ScriptingLib::~ScriptingLib()
	{

	}

	const std::string& ScriptingLib::LibraryName() const
	{
		return m_LibraryName;
	}

	InternalCall::InternalCall(const std::string& language, const std::string& scriptMethod, const void* pRedirectMethod)
		: ScriptExtension(language), m_ScriptMethod(scriptMethod), m_pRedirectMethod(pRedirectMethod)
	{

	}

	const std::string& InternalCall::ScriptMethod() const
	{
		return m_ScriptMethod;
	}

	const void* InternalCall::RedirectMethod() const
	{
		return m_pRedirectMethod;
	}
}
