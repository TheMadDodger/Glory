#include "EntitiesMonoExtender.h"
#include "EntityCSAPI.h"
#include "MonoEntitySceneManager.h"

#include <ScriptingExtender.h>
#include <Engine.h>
#include <MonoSceneManager.h>
#include <ScenesModule.h>
#include <EntitySceneScenesModule.h>

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

	void EntityLibManager::Initialize(Assembly* pAssembly)
	{
		MonoSceneManager::BindImplemetation<MonoEntitySceneManager>();
		MonoEntitySceneManager::Initialize(pAssembly);

		EntitySceneScenesModule* pScenes = (EntitySceneScenesModule*)Game::GetGame().GetEngine()->GetMainModule<ScenesModule>();
		Utils::ECS::ComponentTypes* pComponentTypesInstance = pScenes->ComponentTypesInstance();
		Utils::ECS::ComponentTypes::SetInstance(pComponentTypesInstance);
	}

	void EntityLibManager::Cleanup()
	{
		MonoEntitySceneManager::Cleanup();
		MonoSceneManager::UnbindImplementation();
	}
}
