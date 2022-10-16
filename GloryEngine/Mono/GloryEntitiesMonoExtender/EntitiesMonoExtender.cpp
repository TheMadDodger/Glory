#include "EntitiesMonoExtender.h"
#include "EntitiesMonoExtender.h"
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
	}

	void EntitiesMonoExtender::GetLibs(std::vector<ScriptingLib>& libs)
	{
		libs.push_back(ScriptingLib("csharp", "GloryEngine.Entities.dll"));
	}

	void LoadExtension(Engine* pEngine)
	{
		ScriptingExtender* pExtender = pEngine->GetScriptingExtender();
		pExtender->RegisterExtender<EntitiesMonoExtender>();
	}
}
