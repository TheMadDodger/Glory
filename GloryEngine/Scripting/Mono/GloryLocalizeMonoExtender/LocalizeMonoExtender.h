#pragma once
#include <IScriptExtender.h>
#include <Glory.h>
#include <IMonoLibManager.h>

#include <mono/metadata/object-forward.h>

namespace Glory
{
	class IEngine;
	class Module;

	extern "C" GLORY_API bool OnLoadExtra(const char* path, Module* pModule, Module* pRequiredModule);

	class LocalizeLibManager : public IMonoLibManager
	{
	public: /* Lib manager implementation */
		virtual void CollectTypes(IEngine*, Assembly*) override;
		virtual void Initialize(IEngine* pEngine, Assembly* pAssembly) override;
		virtual void Cleanup(IEngine*) override;
		virtual void Reset(IEngine*) override;

	private:
		Assembly* m_pAssembly;
	};

	class LocalizeMonoExtender : public IScriptExtender
	{
	public:
		LocalizeMonoExtender(const char* path);
		virtual ~LocalizeMonoExtender();

	public: /* Script extension implementation */
		virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
		virtual void GetLibs(ScriptingExtender* pScriptingExtender) override;

	private:
		LocalizeLibManager* m_pLibManager;
		std::string m_Path;
	};
}
