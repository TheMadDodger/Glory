#pragma once
#include <string>

namespace Glory
{
	class ScriptExtension
	{
	public:
		ScriptExtension(const std::string& language);
		virtual ~ScriptExtension();

		const std::string& Language() const;

	private:
		friend class ScriptingExtender;
		const std::string m_Language;
	};

	class ScriptingLib : public ScriptExtension
	{
	public:
		ScriptingLib(const std::string& language, const std::string& libraryName, const std::string& location, bool reloadable, void* data = nullptr, bool isMainLib = false);
		virtual ~ScriptingLib();

		const std::string& LibraryName() const;
		const std::string& Location() const;
		bool Reloadable() const;
		bool IsMainLib() const;
		const void* Data() const;

	private:
		const std::string m_LibraryName;
		const std::string m_Location;
		const bool m_Reloadable;
		const bool m_MainLib;
		const void* m_Data;
	};

	class InternalCall : public ScriptExtension
	{
	public:
		InternalCall(const std::string& language, const std::string& scriptMethod, const void* pRedirectMethod);

		const std::string& ScriptMethod() const;
		const void* RedirectMethod() const;

	private:
		friend class ScriptingExtender;
		std::string m_ScriptMethod;
		const void* m_pRedirectMethod;
	};
}
