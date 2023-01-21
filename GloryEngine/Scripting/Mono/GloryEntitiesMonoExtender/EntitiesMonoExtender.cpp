#include "EntitiesMonoExtender.h"
#include "EntityCSAPI.h"
#include "MonoEntitySceneManager.h"

#include <ScriptingExtender.h>
#include <Engine.h>
#include <MonoSceneManager.h>

namespace Glory
{
	EntitiesMonoExtender::EntitiesMonoExtender() : m_pLibManager(new EntityLibManager())
	{
	}

	EntitiesMonoExtender::~EntitiesMonoExtender()
	{
		delete m_pLibManager;
		m_pLibManager = nullptr;
	}

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
		pScriptingExtender->AddInternalLib("GloryEngine.Entities.dll", m_pLibManager);
	}

	IScriptExtender* OnLoadExtension(Glory::GloryContext* pContext)
	{
		GloryContext::SetContext(pContext);
		return new EntitiesMonoExtender();
	}

	void EntityLibManager::Initialize(AssemblyBinding* pAssembly)
	{
		MonoSceneManager::BindImplemetation<MonoEntitySceneManager>();
		MonoEntitySceneManager::Initialize(pAssembly);
	}

	void EntityLibManager::Cleanup()
	{
		MonoEntitySceneManager::Cleanup();
		MonoSceneManager::UnbindImplementation();
	}
}
