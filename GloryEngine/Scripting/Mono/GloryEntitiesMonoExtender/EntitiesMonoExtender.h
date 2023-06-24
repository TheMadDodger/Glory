#pragma once
#include <IScriptExtender.h>
#include <Glory.h>
#include <GloryContext.h>
#include <IMonoLibManager.h>

namespace Glory
{
	class Engine;

	extern "C" GLORY_API IScriptExtender* OnLoadExtension(Glory::GloryContext* pContext);

	class EntityLibManager : public IMonoLibManager
	{
	public: /* Lib manager implementation */
		virtual void Initialize(Assembly* pAssembly) override;
		virtual void Cleanup() override;
	};

	class EntitiesMonoExtender : public IScriptExtender
	{
	public:
		EntitiesMonoExtender();
		virtual ~EntitiesMonoExtender();

	public: /* Script extension implementation */
		virtual std::string Language() override;
		virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
		virtual void GetLibs(ScriptingExtender* pScriptingExtender) override;

	private:
		EntityLibManager* m_pLibManager;
	};
}
