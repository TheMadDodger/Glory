#pragma once
#include "ScriptLoaderModule.h"
#include "Script.h"

namespace Glory
{
	class ScriptingModule : public Module
	{
	public:
		ScriptingModule();
		virtual ~ScriptingModule();

		virtual LoaderModule* CreateLoaderModule() = 0;
		virtual size_t GetScriptTypeHash() = 0;

	protected:
		virtual void Initialize() {};
		virtual void Cleanup() {};

	private:
		virtual const std::type_info& GetModuleType() override;
	};

	template<class TScript, class TLoader>
	class ScriptingModuleTemplate : public ScriptingModule
	{
	public:
		ScriptingModuleTemplate() : m_ScriptTypeHash(ResourceType::GetHash<TScript>()) {}
		virtual ~ScriptingModuleTemplate() {}

		virtual LoaderModule* CreateLoaderModule() override
		{
			return new TLoader();
		}

		virtual size_t GetScriptTypeHash() override
		{
			return m_ScriptTypeHash;
		}

	private:
		size_t m_ScriptTypeHash;
	};
}
