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

	ScriptingLib::ScriptingLib(const std::string& language, const std::string& libraryName, const std::string& location, bool reloadable, void* data, bool isMainLib)
		: ScriptExtension(language), m_LibraryName(libraryName), m_Location(location), m_Reloadable(reloadable), m_Data(data), m_MainLib(isMainLib)
	{

	}

	ScriptingLib::~ScriptingLib()
	{

	}

	const std::string& ScriptingLib::LibraryName() const
	{
		return m_LibraryName;
	}

	const std::string& ScriptingLib::Location() const
	{
		return m_Location;
	}

	bool ScriptingLib::Reloadable() const
	{
		return m_Reloadable;
	}

	bool ScriptingLib::IsMainLib() const
	{
		return m_MainLib;
	}

	const void* ScriptingLib::Data() const
	{
		return m_Data;
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
