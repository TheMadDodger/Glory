#include "EntitiesMonoExtender.h"
#include "EntityCSAPI.h"
#include <ScriptingExtender.h>
#include <Engine.h>

namespace Glory
{
	std::string EntitiesMonoExtender::Language()
	{
		return "csharp";
	}

	void EntitiesMonoExtender::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		EntityCSAPI::GetInternallCalls(internalCalls);
	}

	void EntitiesMonoExtender::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		pScriptingExtender->AddInternalLib("GloryEngine.Entities.dll");
	}

	IScriptExtender* OnLoadExtension(Glory::GloryContext* pContext)
	{
		GloryContext::SetContext(pContext);
		return new EntitiesMonoExtender();
	}
}
