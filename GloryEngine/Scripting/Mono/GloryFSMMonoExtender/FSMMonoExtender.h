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

	class FSMLibManager : public IMonoLibManager
	{
	public: /* Lib manager implementation */
		virtual void CollectTypes(Engine*, Assembly*) override;
		virtual void Initialize(Engine* pEngine, Assembly* pAssembly) override;
		virtual void Cleanup(Engine*) override;
		virtual void Reset(Engine*) override;

	private:
		Assembly* m_pAssembly;
		MonoObject* m_pFSMManagerObject;
		MonoMethod* m_pFSMReset;
		uint32_t m_FSMGCHandle;
	};

	class FSMMonoExtender : public IScriptExtender
	{
	public:
		FSMMonoExtender(const char* path);
		virtual ~FSMMonoExtender();

	public: /* Script extension implementation */
		virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
		virtual void GetLibs(ScriptingExtender* pScriptingExtender) override;

	private:
		FSMLibManager* m_pLibManager;
		std::string m_Path;
	};
}
