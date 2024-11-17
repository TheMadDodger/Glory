#include "MonoScriptedSystem.h"
#include "GScene.h"

#include "Engine.h"
#include "SceneManager.h"
#include "AssetManager.h"
#include "MonoComponents.h"
#include "MonoManager.h"
#include "CoreLibManager.h"

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
		pComponent.m_pScriptObject = pCoreLibManager->GetScript(sceneID, entityUuid, pComponent.m_CachedComponentID);

		scriptManager.ReadDefaults((size_t)typeIndex, pComponent.m_ScriptData.m_Buffer);
		scriptManager.SetPropertyValues((size_t)typeIndex, pComponent.m_pScriptObject, pComponent.m_ScriptData.m_Buffer);
		scriptManager.Invoke((size_t)typeIndex, pComponent.m_pScriptObject, "Start", nullptr);
	}

	void MonoScriptedSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptManager& scriptManager = MonoManager::Instance()->GetCoreLibManager()->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		scriptManager.Invoke((size_t)typeIndex, pComponent.m_pScriptObject, "Stop", nullptr);
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
		pComponent.m_pScriptObject = pCoreLibManager->CreateScript((size_t)typeIndex, sceneID, entityUuid, pComponent.m_CachedComponentID);
		scriptManager.ReadDefaults((size_t)typeIndex, pComponent.m_ScriptData.m_Buffer);
		scriptManager.SetPropertyValues((size_t)typeIndex, pComponent.m_pScriptObject, pComponent.m_ScriptData.m_Buffer);
		scriptManager.Invoke((size_t)typeIndex, pComponent.m_pScriptObject, "OnValidate", nullptr);
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

		scriptManager.Invoke((size_t)typeIndex, pComponent.m_pScriptObject, "OnEnable", nullptr);
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

		scriptManager.Invoke((size_t)typeIndex, pComponent.m_pScriptObject, "OnDisable", nullptr);
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

		scriptManager.Invoke((size_t)typeIndex, pComponent.m_pScriptObject, "Update", nullptr);
		scriptManager.GetPropertyValues((size_t)typeIndex, pComponent.m_pScriptObject, pComponent.m_ScriptData.m_Buffer);
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

		scriptManager.Invoke((size_t)typeIndex, pComponent.m_pScriptObject, "Draw", nullptr);
	}

	void MonoScriptedSystem::OnBodyActivated(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t bodyID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		std::vector<void*> args = {
			&bodyID
		};
		scriptManager.Invoke((size_t)typeIndex, scriptComponent.m_pScriptObject, "OnBodyActivated", args.data());
	}

	void MonoScriptedSystem::OnBodyDeactivated(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t bodyID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		std::vector<void*> args = {
			&bodyID
		};
		scriptManager.Invoke((size_t)typeIndex, scriptComponent.m_pScriptObject, "OnBodyDeactivated", args.data());
	}

	void MonoScriptedSystem::OnContactAdded(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		std::vector<void*> args = {
			&body1ID,
			&body2ID
		};
		scriptManager.Invoke((size_t)typeIndex, scriptComponent.m_pScriptObject, "OnContactAdded", args.data());
	}

	void MonoScriptedSystem::OnContactPersisted(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		std::vector<void*> args = {
			&body1ID,
			&body2ID
		};
		scriptManager.Invoke((size_t)typeIndex, scriptComponent.m_pScriptObject, "OnContactPersisted", args.data());
	}

	void MonoScriptedSystem::OnContactRemoved(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, uint32_t body1ID, uint32_t body2ID)
	{
		if (!pRegistry->HasComponent<MonoScriptComponent>(entity)) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AssetManager* pAssets = &pEngine->GetAssetManager();

		MonoScriptComponent& scriptComponent = pRegistry->GetComponent<MonoScriptComponent>(entity);

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		std::vector<void*> args = {
			&body1ID,
			&body2ID
		};
		scriptManager.Invoke((size_t)typeIndex, scriptComponent.m_pScriptObject, "OnContactRemoved", args.data());
	}
}
