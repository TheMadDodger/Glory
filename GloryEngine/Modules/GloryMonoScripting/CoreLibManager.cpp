#include "CoreLibManager.h"
#include "MonoManager.h"
#include "ScriptingMethodsHelper.h"
#include "Assembly.h"
#include "AssemblyDomain.h"

#include <Engine.h>
#include <Debug.h>
#include <SceneManager.h>

namespace Glory
{
	CoreLibManager::CoreLibManager(MonoManager* pMonoManager):
		m_pMonoManager(pMonoManager), m_pAssembly(nullptr), m_pEngineObject(nullptr), m_pEngineReset(nullptr),
		m_SceneClosingCallback(0), m_SceneObjectDestroyedCallback(0), m_EngineGCHandle(0), m_ScriptManager(this)
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

		if (!m_SceneClosingCallback)
		{
			m_SceneClosingCallback = pEngine->GetSceneManager()->AddSceneClosingCallback([this](UUID sceneID, UUID) {
				OnSceneDestroy(sceneID);
			});
		}

		if (!m_SceneObjectDestroyedCallback)
		{
			m_SceneObjectDestroyedCallback = pEngine->GetSceneManager()->AddSceneObjectDestroyedCallback([this](UUID sceneID, UUID objectID) {
				OnSceneObjectDestroy(objectID, sceneID);
			});
		}

		Utils::ECS::ComponentTypes::SetInstance(pEngine->GetSceneManager()->ComponentTypesInstance());
	}

	void CoreLibManager::Cleanup(Engine* pEngine)
	{
		if (m_SceneClosingCallback) pEngine->GetSceneManager()->RemoveSceneClosingCallback(m_SceneClosingCallback);
		if (m_SceneObjectDestroyedCallback) pEngine->GetSceneManager()->RemoveSceneObjectDestroyedCallback(m_SceneObjectDestroyedCallback);

		m_SceneClosingCallback = 0;
		m_SceneObjectDestroyedCallback = 0;
	}

	void CoreLibManager::Reset(Engine* pEngine)
	{
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");
		MonoMethod* pReset = pEngineClass->GetMethod(".::Reset");
		MonoObject* pExcept;
		MonoObject* pReturn = mono_runtime_invoke(pReset, m_pEngineObject, nullptr, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
	}

	MonoObject* CoreLibManager::CreateAssetObject(UUID uuid, const std::string_view type)
	{
		if (!uuid) return nullptr;
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");
		MonoMethod* pCreate = pEngineClass->GetMethod(".::MakeResource");
		MonoString* pMonoString = mono_string_new(mono_domain_get(), type.data());
		void* args[2] = { &uuid, (void*)pMonoString };
		MonoObject* pExcept;
		MonoObject* pReturn = mono_runtime_invoke(pCreate, m_pEngineObject, args, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
		return pReturn;
	}

	MonoObject* CoreLibManager::GetScriptDummy(MonoClass* pClass)
	{
		MonoType* pType = mono_class_get_type(pClass);
		char* pTypeName = mono_type_get_name(pType);
		int index = m_ScriptManager.TypeIndexFromName(pTypeName);
		mono_free(pTypeName);
		if (index == -1) return nullptr;
		return m_ScriptManager.Dummy((size_t)index);
	}
	
	MonoObject* CoreLibManager::CreateScript(size_t typeIndex, UUID sceneID, UUID objectID, UUID componentID)
	{
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");
		MonoMethod* pCreate = pEngineClass->GetMethod(".::CreateScript");
		void* args[4] = { &typeIndex, &sceneID, &objectID, &componentID };
		MonoObject* pExcept;
		MonoObject* pReturn = mono_runtime_invoke(pCreate, m_pEngineObject, args, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
		return pReturn;
	}

	MonoObject* CoreLibManager::GetScript(UUID sceneID, UUID objectID, UUID componentID)
	{
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");
		MonoMethod* pGetScript = pEngineClass->GetMethod(".::GetScript");
		void* args[3] = { &sceneID, &objectID, &componentID };
		MonoObject* pExcept;
		MonoObject* pReturn = mono_runtime_invoke(pGetScript, m_pEngineObject, args, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
		return pReturn;
	}

	const MonoScriptManager& CoreLibManager::ScriptManager() const
	{
		return m_ScriptManager;
	}

	MonoScriptManager& CoreLibManager::ScriptManager()
	{
		return m_ScriptManager;
	}

	MonoObject* CoreLibManager::GetEngine()
	{
		return m_pEngineObject;
	}

	MonoObject* CoreLibManager::CreateSceneObject(UUID objectID, UUID sceneID)
	{
		if (!objectID || !sceneID) return nullptr;
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");
		MonoMethod* pCreate = pEngineClass->GetMethod(".::MakeSceneObject");
		void* args[2] = { &objectID, &sceneID };
		MonoObject* pExcept;
		MonoObject* pReturn = mono_runtime_invoke(pCreate, m_pEngineObject, args, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
		return pReturn;
	}

	void CoreLibManager::CreateEngine(Engine* pEngine)
	{
		if (m_EngineGCHandle)
		{
			mono_gchandle_free(m_EngineGCHandle);
			m_pMonoManager->CollectGC();
			m_pMonoManager->WaitForPendingFinalizers();
			m_EngineGCHandle = 0;
		}

		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");

		m_pEngineObject = mono_object_new(mono_domain_get(), pEngineClass->m_pClass);
		if (m_pEngineObject == nullptr)
		{
			pEngine->GetDebug().LogError("CoreLibManager::CreateEngine > Failed to create engine MonoObject");
			return;
		}

		mono_runtime_object_init(m_pEngineObject);
		m_pEngineReset = pEngineClass->GetMethod(".::Reset");
		m_EngineGCHandle = mono_gchandle_new(m_pEngineObject, true);

		m_ScriptManager.Initialize(pEngineClass, m_pEngineObject);
	}

	void CoreLibManager::OnSceneDestroy(UUID sceneID)
	{
		if (!sceneID) return;
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");
		MonoMethod* pCreate = pEngineClass->GetMethod(".::OnSceneDestroy");
		void* args[1] = { &sceneID };
		MonoObject* pExcept;
		mono_runtime_invoke(pCreate, m_pEngineObject, args, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
	}

	void CoreLibManager::OnSceneObjectDestroy(UUID objectID, UUID sceneID)
	{
		if (!objectID || !sceneID) return;
		AssemblyClass* pEngineClass = m_pAssembly->GetClass("GloryEngine", "Engine");
		MonoMethod* pCreate = pEngineClass->GetMethod(".::OnSceneObjectDestroy");
		void* args[2] = { &objectID, &sceneID };
		MonoObject* pExcept;
		mono_runtime_invoke(pCreate, m_pEngineObject, args, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
	}
}
