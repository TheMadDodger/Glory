#include "ScriptedSystem.h"
#include "EntityScene.h"

#include <Engine.h>
#include <ScenesModule.h>
#include <AssetManager.h>

namespace Glory
{
	void ScriptedSystem::OnAdd(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
	}

	void ScriptedSystem::OnStart(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
		pScript->SetPropertyValues(pObject, pComponent.m_ScriptData);

		pScript->Invoke(pObject, "Start", nullptr);
	}

	void ScriptedSystem::OnStop(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Stop", nullptr);
	}

	void ScriptedSystem::OnValidate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);

		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->SetPropertyValues(pObject, pComponent.m_ScriptData);
		pScript->Invoke(pObject, "OnValidate", nullptr);
	}

	void ScriptedSystem::OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Update", nullptr);
		pScript->GetPropertyValues(pObject, pComponent.m_ScriptData);
	}

	void ScriptedSystem::OnDraw(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Draw", nullptr);
	}
}
