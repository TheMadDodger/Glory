#include "EntityLibManager.h"
#include "EntityCSAPI.h"
#include "MonoSceneManager.h"
#include "MonoSceneManager.h"

#include <ScriptingExtender.h>
#include <Engine.h>
#include <SceneManager.h>

namespace Glory
{
	void EntityLibManager::Initialize(Assembly* pAssembly)
	{
		MonoSceneManager::Initialize(pAssembly);

		SceneManager* pScenes = Game::GetGame().GetEngine()->GetSceneManager();
		Utils::ECS::ComponentTypes* pComponentTypesInstance = pScenes->ComponentTypesInstance();
		Utils::ECS::ComponentTypes::SetInstance(pComponentTypesInstance);
	}

	void EntityLibManager::Cleanup()
	{
		MonoSceneManager::Cleanup();
	}
}
