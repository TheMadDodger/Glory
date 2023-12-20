#include "CoreLibManager.h"
#include "MonoAssetManager.h"
#include "MonoManager.h"
#include "ScriptingMethodsHelper.h"
#include "Assembly.h"
#include "MonoSceneManager.h"

#include <Engine.h>
#include <SceneManager.h>

namespace Glory
{
	CoreLibManager::CoreLibManager(MonoManager* pMonoManager) : m_pMonoManager(pMonoManager), m_pAssembly(nullptr)
	{
	}

	Assembly* CoreLibManager::GetAssemblyBinding() const
	{
		return m_pAssembly;
	}

	void CoreLibManager::Initialize(Engine* pEngine, Assembly* pAssembly)
	{
		m_pAssembly = pAssembly;
		MonoAssetManager::Initialize(m_pAssembly->GetMonoImage());
		m_pMonoManager->GetMethodsHelper()->Initialize(m_pAssembly->GetMonoImage());

		/* Pre-load Object classes */
		m_pAssembly->GetClass("GloryEngine", "Object");
		m_pAssembly->GetClass("GloryEngine.SceneManagement", "SceneObject");

		MonoSceneManager::Initialize(pEngine, pAssembly);

		Utils::ECS::ComponentTypes::SetInstance(pEngine->GetSceneManager()->ComponentTypesInstance());
	}

	void CoreLibManager::Cleanup()
	{
		MonoSceneManager::Cleanup();
	}
}
