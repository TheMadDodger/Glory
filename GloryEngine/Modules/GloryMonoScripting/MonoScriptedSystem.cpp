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
}
