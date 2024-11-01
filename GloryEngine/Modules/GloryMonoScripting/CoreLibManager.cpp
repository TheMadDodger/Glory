#include "CoreLibManager.h"
#include "MonoManager.h"
#include "ScriptingMethodsHelper.h"
#include "Assembly.h"
#include "MonoSceneManager.h"
#include "MonoComponentObjectManager.h"

#include <Engine.h>
#include <Debug.h>
#include <SceneManager.h>

namespace Glory
{
	CoreLibManager::CoreLibManager(MonoManager* pMonoManager):
		m_pMonoManager(pMonoManager), m_pAssembly(nullptr), m_pEngineObject(nullptr), m_pEngineReset(nullptr)
	{
	}

	Assembly* CoreLibManager::GetAssemblyBinding() const
	{
		return m_pAssembly;
	}

	void CoreLibManager::Initialize(Engine* pEngine, Assembly* pAssembly)
	{
		m_pAssembly = pAssembly;
		CreateEngine(pEngine);

		m_pMonoManager->GetMethodsHelper()->Initialize(m_pAssembly->GetMonoImage());

		/* Pre-load Object classes */
		m_pAssembly->GetClass("GloryEngine", "Object");
		m_pAssembly->GetClass("GloryEngine.SceneManagement", "SceneObject");

		MonoSceneManager::Initialize(pEngine, pAssembly);
		MonoComponentObjectManager::Initialize(m_pAssembly->GetMonoImage());

		Utils::ECS::ComponentTypes::SetInstance(pEngine->GetSceneManager()->ComponentTypesInstance());
	}

	void CoreLibManager::Cleanup()
	{
		MonoSceneManager::Cleanup();
		MonoComponentObjectManager::Cleanup();
	}

	MonoObject* CoreLibManager::CreateAssetObject(UUID uuid, const std::string_view type)
	{
		if (!uuid) return nullptr;
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");
		MonoMethod* pCreate = pEngineClass->GetMethod(".::MakeResource");
		MonoString* pMonoString = mono_string_new(mono_domain_get(), type.data());
		void* args[2] = { &uuid, (void*)pMonoString };
		MonoObject* pExcept;
		return mono_runtime_invoke(pCreate, m_pEngineObject, args, &pExcept);
		/* TODO: Handle exception? */
	}

	void CoreLibManager::CreateEngine(Engine* pEngine)
	{
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");

		m_pEngineObject = mono_object_new(mono_domain_get(), pEngineClass->m_pClass);
		if (m_pEngineObject == nullptr)
		{
			pEngine->GetDebug().LogError("CoreLibManager::CreateEngine > Failed to create engine MonoObject");
			return;
		}

		mono_runtime_object_init(m_pEngineObject);
		m_pEngineReset = pEngineClass->GetMethod(".::Reset");
	}
}
