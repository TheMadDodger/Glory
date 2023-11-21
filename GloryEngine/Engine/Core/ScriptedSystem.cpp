#include "ScriptedSystem.h"
#include "GScene.h"

#include "Engine.h"
#include "SceneManager.h"
#include "AssetManager.h"

namespace Glory
{
	void ScriptedSystem::OnAdd(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
	}

	void ScriptedSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		const UUID entityUuid = pScene->GetEntityUUID(entity);

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
		pScript->SetPropertyValues(entityUuid, pComponent.m_ScriptData);

		pScript->Invoke(entityUuid, "Start", nullptr);
	}

	void ScriptedSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		pScript->Invoke(entityUuid, "Stop", nullptr);
	}

	void ScriptedSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		pScript->SetPropertyValues(entityUuid, pComponent.m_ScriptData);
		pScript->Invoke(entityUuid, "OnValidate", nullptr);
	}

	void ScriptedSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		pScript->Invoke(entityUuid, "Update", nullptr);
		pScript->GetPropertyValues(entityUuid, pComponent.m_ScriptData);
	}

	void ScriptedSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		pScript->Invoke(entityUuid, "Draw", nullptr);
	}
}
