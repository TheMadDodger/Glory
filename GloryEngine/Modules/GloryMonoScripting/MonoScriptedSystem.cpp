#include "MonoScriptedSystem.h"
#include "GScene.h"

#include "Engine.h"
#include "SceneManager.h"
#include "AssetManager.h"
#include "MonoComponents.h"

namespace Glory
{
	void MonoScriptedSystem::OnAdd(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		MonoScript* pScript = pAssets->GetAssetImmediate<MonoScript>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
	}

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

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
		const UUID sceneID = pScene->GetUUID();
		pScript->SetPropertyValues(entityUuid, sceneID, pComponent.m_ScriptData);

		pScript->Invoke(entityUuid, sceneID, "Start", nullptr);
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
		pScript->Invoke(entityUuid, sceneID, "Stop", nullptr);
	}

	void MonoScriptedSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		MonoScript* pScript = pAssets->GetAssetImmediate<MonoScript>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);

		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		pScript->SetPropertyValues(entityUuid, sceneID, pComponent.m_ScriptData);
		pScript->Invoke(entityUuid, sceneID, "OnValidate", nullptr);
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
		pScript->Invoke(entityUuid, sceneID, "Update", nullptr);
		pScript->GetPropertyValues(entityUuid, sceneID, pComponent.m_ScriptData);
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
		pScript->Invoke(entityUuid, sceneID, "Draw", nullptr);
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

		pScript->Invoke(pScene->GetEntityUUID(entity), pScene->GetUUID(), "OnBodyActivated", args.data());
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

		pScript->Invoke(pScene->GetEntityUUID(entity), pScene->GetUUID(), "OnBodyDeactivated", args.data());
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
		pScript->Invoke(pScene->GetEntityUUID(entity), pScene->GetUUID(), "OnContactAdded", args.data());
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
		pScript->Invoke(pScene->GetEntityUUID(entity), pScene->GetUUID(), "OnContactPersisted", args.data());
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
		pScript->Invoke(pScene->GetEntityUUID(entity), pScene->GetUUID(), "OnContactRemoved", args.data());
	}
}
