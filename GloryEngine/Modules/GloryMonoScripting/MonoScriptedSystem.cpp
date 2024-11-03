#include "MonoScriptedSystem.h"
#include "GScene.h"

#include "Engine.h"
#include "SceneManager.h"
#include "AssetManager.h"
#include "MonoComponents.h"

namespace Glory
{
	void MonoScriptedSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		MonoScript* pScript = pAssets->GetAssetImmediate<MonoScript>(uuid);
		if (pScript == nullptr) return;
		const UUID entityUuid = pScene->GetEntityUUID(entity);

		pScript->LoadScriptProperties();
		pScript->GetScriptProperties(pComponent.m_ScriptProperties);
		pScript->ReadDefaults(pComponent.m_ScriptData.m_Buffer);
		const UUID sceneID = pScene->GetUUID();
		pScript->SetPropertyValues(pComponent.m_pScriptObject, pComponent.m_ScriptData.m_Buffer);

		pScript->Invoke(pComponent.m_pScriptObject, "Start", nullptr);
	}

	void MonoScriptedSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		MonoScript* pScript = pAssets->GetOrLoadAsset<MonoScript>(uuid);
		if (pScript == nullptr) return;
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pRegistry->GetUserData<GScene*>()->GetUUID();
		pScript->Invoke(pComponent.m_pScriptObject, "Stop", nullptr);
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

		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		MonoScript* pScript = pAssets->GetAssetImmediate<MonoScript>(uuid);
		if (pScript == nullptr) return;

		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		pComponent.m_pScriptObject = pScript->CreateScriptObject(entityUuid, sceneID, pComponent.m_CachedComponentID);

		pScript->LoadScriptProperties();
		pScript->GetScriptProperties(pComponent.m_ScriptProperties);
		pScript->ReadDefaults(pComponent.m_ScriptData.m_Buffer);
		pScript->SetPropertyValues(pComponent.m_pScriptObject, pComponent.m_ScriptData.m_Buffer);
		pScript->Invoke(pComponent.m_pScriptObject, "OnValidate", nullptr);
	}

	void MonoScriptedSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		MonoScript* pScript = pAssets->GetOrLoadAsset<MonoScript>(uuid);
		if (pScript == nullptr) return;
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		pScript->Invoke(pComponent.m_pScriptObject, "Update", nullptr);
		pScript->GetPropertyValues(pComponent.m_pScriptObject, pComponent.m_ScriptData.m_Buffer);
	}

	void MonoScriptedSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		MonoScript* pScript = pAssets->GetOrLoadAsset<MonoScript>(uuid);
		if (pScript == nullptr) return;
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		pScript->Invoke(pComponent.m_pScriptObject, "Draw", nullptr);
	}

	void MonoScriptedSystem::OnBodyActivated(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t bodyID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);
		MonoScript* pScript = scriptComponent.m_Script.Get(pAssets);

		if (!pScript) return;
		std::vector<void*> args = {
			&bodyID
		};

		pScript->Invoke(scriptComponent.m_pScriptObject, "OnBodyActivated", args.data());
	}

	void MonoScriptedSystem::OnBodyDeactivated(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t bodyID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);
		MonoScript* pScript = scriptComponent.m_Script.Get(pAssets);

		if (!pScript) return;
		std::vector<void*> args = {
			&bodyID
		};

		pScript->Invoke(scriptComponent.m_pScriptObject, "OnBodyDeactivated", args.data());
	}

	void MonoScriptedSystem::OnContactAdded(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);
		MonoScript* pScript = scriptComponent.m_Script.Get(pAssets);
		if (!pScript) return;
		std::vector<void*> args = {
			&body1ID,
			&body2ID
		};
		pScript->Invoke(scriptComponent.m_pScriptObject, "OnContactAdded", args.data());
	}

	void MonoScriptedSystem::OnContactPersisted(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);
		MonoScript* pScript = scriptComponent.m_Script.Get(pAssets);
		if (!pScript) return;
		std::vector<void*> args = {
			&body1ID,
			&body2ID
		};
		pScript->Invoke(scriptComponent.m_pScriptObject, "OnContactPersisted", args.data());
	}

	void MonoScriptedSystem::OnContactRemoved(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);
		MonoScript* pScript = scriptComponent.m_Script.Get(pAssets);
		if (!pScript) return;
		std::vector<void*> args = {
			&body1ID,
			&body2ID
		};
		pScript->Invoke(scriptComponent.m_pScriptObject, "OnContactRemoved", args.data());
	}
}
