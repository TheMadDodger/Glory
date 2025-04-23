#pragma once
#include <IScriptExtender.h>
#include <Glory.h>
#include <IMonoLibManager.h>

#include <mono/metadata/object-forward.h>

namespace Glory
{
	class Engine;
	class Module;

	extern "C" GLORY_API bool OnLoadExtra(const char* path, Module* pModule, Module* pRequiredModule);

	class LocalizeLibManager : public IMonoLibManager
	{
	public: /* Lib manager implementation */
		virtual void CollectTypes(Engine*, Assembly*) override;
		virtual void Initialize(Engine* pEngine, Assembly* pAssembly) override;
		virtual void Cleanup(Engine*) override;
		virtual void Reset(Engine*) override;

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
