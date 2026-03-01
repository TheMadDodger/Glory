#pragma once
#include <IScriptExtender.h>
#include <Glory.h>
#include <IMonoLibManager.h>

namespace Glory
{
	class IEngine;
	class Module;

	extern "C" GLORY_API bool OnLoadExtra(const char* path, Module* pModule, Module* pRequiredModule);

	class JoltLibManager : public IMonoLibManager
	{
	public: /* Lib manager implementation */
		virtual void CollectTypes(IEngine*, Assembly*) override;
		virtual void Initialize(IEngine* pEngine, Assembly* pAssembly) override;
		virtual void Cleanup(IEngine*) override;
		virtual void Reset(IEngine*) override;
	};

	class JoltMonoExtender : public IScriptExtender
	{
	public:
		JoltMonoExtender(const char* path);
		virtual ~JoltMonoExtender();

	public: /* Script extension implementation */
		virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
		virtual void GetLibs(ScriptingExtender* pScriptingExtender) override;

	private:
		JoltLibManager* m_pLibManager;
		std::string m_Path;
	};
}
