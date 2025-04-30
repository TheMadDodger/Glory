#include "MonoScriptedSystem.h"
#include "MonoComponents.h"
#include "MonoManager.h"
#include "CoreLibManager.h"

#include <GScene.h>
#include <Engine.h>
#include <SceneManager.h>
#include <AssetManager.h>
#include <Debug.h>
#include <UUIDRemapper.h>
#include <SceneObjectRef.h>

namespace Glory
{
	void MonoScriptedSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			if (pEntityView->ComponentTypeAt(i) != MonoScriptComponent::GetTypeData()->TypeHash()) continue;
			pComponent.m_CachedComponentID = pEntityView->ComponentUUIDAt(i);
			break;
		}

		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		MonoObject* pScriptObject = pCoreLibManager->GetScript(sceneID, entityUuid, pComponent.m_CachedComponentID);
		pComponent.m_ScriptObjectHandle = mono_gchandle_new_weakref(pScriptObject, false);

		scriptManager.ReadDefaults((size_t)typeIndex, pComponent.m_ScriptData.m_Buffer);
		scriptManager.SetPropertyValues((size_t)typeIndex, pScriptObject, pComponent.m_ScriptData.m_Buffer);
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "Start", nullptr);
	}

	void MonoScriptedSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptManager& scriptManager = MonoManager::Instance()->GetCoreLibManager()->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "Stop", nullptr);
	}

	void MonoScriptedSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			if (pEntityView->ComponentTypeAt(i) != MonoScriptComponent::GetTypeData()->TypeHash()) continue;
			pComponent.m_CachedComponentID = pEntityView->ComponentUUIDAt(i);
			break;
		}

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		MonoObject* pScriptObject = pCoreLibManager->CreateScript((size_t)typeIndex, sceneID, entityUuid, pComponent.m_CachedComponentID);
		pComponent.m_ScriptObjectHandle = mono_gchandle_new_weakref(pScriptObject, false);
		scriptManager.ReadDefaults((size_t)typeIndex, pComponent.m_ScriptData.m_Buffer);
		scriptManager.SetPropertyValues((size_t)typeIndex, pScriptObject, pComponent.m_ScriptData.m_Buffer);
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnValidate", nullptr);
	}

	void MonoScriptedSystem::OnEnable(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnEnable", nullptr);
	}

	void MonoScriptedSystem::OnDisable(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnDisable", nullptr);
	}

	void MonoScriptedSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "Update", nullptr);

		/** @todo: This should be handled by the editor somehow */
		//if (pComponent.m_ScriptData.m_Buffer.empty()) pComponent.m_ScriptData.m_Buffer = std::vector<char>(4, '\0');
		//scriptManager.GetPropertyValues((size_t)typeIndex, pScriptObject, pComponent.m_ScriptData.m_Buffer);
	}

	void MonoScriptedSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "Draw", nullptr);
	}

	void MonoScriptedSystem::OnCopy(GScene* pScene, void* data, UUID componentId, UUIDRemapper& remapper)
	{
		MonoScriptComponent& component = *static_cast<MonoScriptComponent*>(data);
		component.m_ScriptObjectHandle = 0;
		component.m_CachedComponentID = 0;
		if (!component.m_ScriptType.m_Hash) return;
		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		const int typeIndex = scriptManager.TypeIndexFromHash(component.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;
		component.m_CachedComponentID = componentId;
		auto& properties = scriptManager.ScriptProperties(typeIndex);
		for (size_t i = 0; i < properties.size(); ++i)
		{
			auto& prop = properties[i];
			if (prop.m_TypeHash != ST_Object) continue;
			char* idData = &component.m_ScriptData.m_Buffer[prop.m_RelativeOffset];
			SceneObjectRef* id = reinterpret_cast<SceneObjectRef*>(idData);
			const UUID newId = remapper(id->ObjectUUID());
			*id->ObjectUUIDMember() = newId;
			*id->SceneUUIDMember() = pScene->GetUUID();
		}
	}

	void MonoScriptedSystem::GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references)
	{
		for (size_t i = 0; i < pTypeView->Size(); ++i)
		{
			const MonoScriptComponent* pScriptComponent = static_cast<const MonoScriptComponent*>(pTypeView->GetComponentAddressFromIndex(i));
			if (!pScriptComponent->m_ScriptType.m_Hash) continue;
			MonoScriptManager& scripts = MonoManager::Instance()->GetCoreLibManager()->ScriptManager();
			const int typeIndex = scripts.TypeIndexFromHash(pScriptComponent->m_ScriptType.m_Hash);
			if (typeIndex == -1) continue;
			const std::vector<ScriptProperty>& properties = scripts.ScriptProperties(typeIndex);
			for (size_t i = 0; i < properties.size(); ++i)
			{
				if (properties[i].m_TypeHash != ST_Asset) continue;
				const size_t offset = properties[i].m_RelativeOffset;
				if (offset + sizeof(uint64_t) >= pScriptComponent->m_ScriptData.m_Buffer.size()) continue;
				const AssetReferenceBase* pReferenceMember =
					reinterpret_cast<const AssetReferenceBase*>(&pScriptComponent->m_ScriptData.m_Buffer[offset]);
				if (pReferenceMember->AssetUUID()) references.push_back(pReferenceMember->AssetUUID());
			}
		}
	}

	void MonoScriptedSystem::OnBodyActivated(Engine* pEngine, UUID sceneID, UUID entityUUID)
	{
		GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(sceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(entityUUID);
		if (!entity.IsValid()) return;
		if (!entity.HasComponent<MonoScriptComponent>()) return;
		MonoScriptComponent& scriptComponent = entity.GetComponent<MonoScriptComponent>();
		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;
		MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnBodyActivated", nullptr);
	}

	void MonoScriptedSystem::OnBodyDeactivated(Engine* pEngine, UUID sceneID, UUID entityUUID)
	{
		GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(sceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(entityUUID);
		if (!entity.IsValid()) return;
		if (!entity.HasComponent<MonoScriptComponent>()) return;
		MonoScriptComponent& scriptComponent = entity.GetComponent<MonoScriptComponent>();
		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;
		MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnBodyDeactivated", nullptr);
	}

	void MonoScriptedSystem::OnContactAdded(Engine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID)
	{
		GScene* pScene1 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene1) return;
		Entity entity1 = pScene1->GetEntityByUUID(entity1UUID);
		if (!entity1.IsValid()) return;
		GScene* pScene2 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene2) return;
		Entity entity2 = pScene2->GetEntityByUUID(entity2UUID);
		if (!entity2.IsValid()) return;

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		MonoObject* pMonoObject1 = pCoreLibManager->CreateSceneObject(entity1UUID, scene1ID);
		MonoObject* pMonoObject2 = pCoreLibManager->CreateSceneObject(entity2UUID, scene2ID);
		if (!pMonoObject1 || !pMonoObject2) return;

		if (entity1.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity1.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject2
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactAdded", args.data());
			}
		}

		if (entity2.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity2.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject1
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactAdded", args.data());
			}
		}
	}

	void MonoScriptedSystem::OnContactPersisted(Engine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID)
	{
		GScene* pScene1 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene1) return;
		Entity entity1 = pScene1->GetEntityByUUID(entity1UUID);
		if (!entity1.IsValid()) return;
		GScene* pScene2 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene2) return;
		Entity entity2 = pScene2->GetEntityByUUID(entity2UUID);
		if (!entity2.IsValid()) return;

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		MonoObject* pMonoObject1 = pCoreLibManager->CreateSceneObject(entity1UUID, scene1ID);
		MonoObject* pMonoObject2 = pCoreLibManager->CreateSceneObject(entity2UUID, scene2ID);
		if (!pMonoObject1 || !pMonoObject2) return;

		if (entity1.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity1.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject2
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactPersisted", args.data());
			}
		}

		if (entity2.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity2.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject1
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactPersisted", args.data());
			}
		}
	}

	void MonoScriptedSystem::OnContactRemoved(Engine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID)
	{
		GScene* pScene1 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene1) return;
		Entity entity1 = pScene1->GetEntityByUUID(entity1UUID);
		if (!entity1.IsValid()) return;
		GScene* pScene2 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene2) return;
		Entity entity2 = pScene2->GetEntityByUUID(entity2UUID);
		if (!entity2.IsValid()) return;

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		MonoObject* pMonoObject1 = pCoreLibManager->CreateSceneObject(entity1UUID, scene1ID);
		MonoObject* pMonoObject2 = pCoreLibManager->CreateSceneObject(entity2UUID, scene2ID);
		if (!pMonoObject1 || !pMonoObject2) return;

		if (entity1.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity1.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject2
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactRemoved", args.data());
			}
		}

		if (entity2.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity2.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject1
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactRemoved", args.data());
			}
		}
	}
}
