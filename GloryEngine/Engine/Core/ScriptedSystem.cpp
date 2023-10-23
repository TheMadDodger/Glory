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
		SceneManager* pScenes = Game::GetGame().GetEngine()->GetSceneManager();
		SceneObject* pObject = pScenes->GetSceneObjectFromObjectID(entity);

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
		pScript->SetPropertyValues(pObject, pComponent.m_ScriptData);

		pScript->Invoke(pObject, "Start", nullptr);
	}

	void ScriptedSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		SceneManager* pScenes = Game::GetGame().GetEngine()->GetSceneManager();
		SceneObject* pObject = pScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Stop", nullptr);
	}

	void ScriptedSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);

		SceneManager* pScenes = Game::GetGame().GetEngine()->GetSceneManager();
		SceneObject* pObject = pScenes->GetSceneObjectFromObjectID(entity);
		pScript->SetPropertyValues(pObject, pComponent.m_ScriptData);
		pScript->Invoke(pObject, "OnValidate", nullptr);
	}

	void ScriptedSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		SceneManager* pScenes = Game::GetGame().GetEngine()->GetSceneManager();
		SceneObject* pObject = pScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Update", nullptr);
		pScript->GetPropertyValues(pObject, pComponent.m_ScriptData);
	}

	void ScriptedSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		SceneManager* pScenes = Game::GetGame().GetEngine()->GetSceneManager();
		SceneObject* pObject = pScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Draw", nullptr);
	}
}
