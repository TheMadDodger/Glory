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
		std::string m_Language;
	};

	class ScriptingLib : public ScriptExtension
	{
	public:
		ScriptingLib(const std::string& language, const std::string& libraryName);
		virtual ~ScriptingLib();

		const std::string& LibraryName() const;

	private:
		std::string m_LibraryName;
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
