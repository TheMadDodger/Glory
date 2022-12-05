#pragma once
#include <IScriptExtender.h>
#include <Glory.h>
#include <GloryContext.h>

namespace Glory
{
	class Engine;

	extern "C" GLORY_API IScriptExtender* OnLoadExtension(Glory::GloryContext* pContext);

	class EntitiesMonoExtender : public IScriptExtender
	{
	public:
		virtual std::string Language() override;
		virtual void GetInternalCalls(std::vector<InternalCall>& internalCalls) override;
		virtual void GetLibs(ScriptingExtender* pScriptingExtender) override;
	};
}
