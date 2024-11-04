#include "ScriptExtensions.h"

namespace Glory
{
	ScriptExtension::ScriptExtension()
	{
	}

	ScriptExtension::~ScriptExtension()
	{
	}

	ScriptingLib::ScriptingLib(const std::string& libraryName, const std::string& location, bool reloadable, void* data)
		: m_LibraryName(libraryName), m_Location(location), m_Reloadable(reloadable), m_Data(data)
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

	const void* ScriptingLib::Data() const
	{
		return m_Data;
	}

	InternalCall::InternalCall(const std::string& scriptMethod, const void* pRedirectMethod)
		: m_ScriptMethod(scriptMethod), m_pRedirectMethod(pRedirectMethod)
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
