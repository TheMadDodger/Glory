#pragma once
#include <IScriptExtender.h>
#include <Glory.h>
#include <IMonoLibManager.h>

namespace Glory
{
	class Engine;

	extern "C" GLORY_API IScriptExtender* OnLoadExtension();

	class JoltLibManager : public IMonoLibManager
	{
	public: /* Lib manager implementation */
		virtual void Initialize(Engine* pEngine, Assembly* pAssembly) override;
		virtual void Cleanup() override;
	};

	class JoltMonoExtender : public IScriptExtender
	{
	public:
		JoltMonoExtender();
		virtual ~JoltMonoExtender();

	public: /* Script extension implementation */
		virtual std::string Language() override;
		virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
		virtual void GetLibs(ScriptingExtender* pScriptingExtender) override;

	private:
		JoltLibManager* m_pLibManager;
	};
}
