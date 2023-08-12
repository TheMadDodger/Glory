#pragma once
#include "ScriptLoaderModule.h"
#include "Script.h"

namespace Glory
{
	class ScriptingLib;
	class InternalCall;

	class ScriptingModule : public Module
	{
	public:
		ScriptingModule(const std::string& scriptingLanguage);
		virtual ~ScriptingModule();

		virtual LoaderModule* CreateLoaderModule() = 0;
		virtual uint32_t GetScriptTypeHash() = 0;

		virtual const std::string& ScriptingLanguage();

	protected:
		virtual void Initialize() {};
		virtual void Cleanup() {};

		virtual void AddLib(const ScriptingLib& library) = 0;
		virtual void Bind(const InternalCall& internalCall) = 0;

	private:
		virtual const std::type_info& GetModuleType() override;

	private:
		friend class ScriptingExtender;
		const std::string m_ScriptingLanguage;
	};

	template<class TScript, class TLoader>
	class ScriptingModuleTemplate : public ScriptingModule
	{
	public:
		ScriptingModuleTemplate(const std::string& scriptingLanguage) : ScriptingModule(scriptingLanguage), m_ScriptTypeHash(ResourceType::GetHash<TScript>()) {}
		virtual ~ScriptingModuleTemplate() {}

		virtual LoaderModule* CreateLoaderModule() override
		{
			return new TLoader();
		}

		virtual uint32_t GetScriptTypeHash() override
		{
			return m_ScriptTypeHash;
		}

	private:
		uint32_t m_ScriptTypeHash;
	};
}
