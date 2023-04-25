#include "ScriptedSystem.h"
#include "EntityScene.h"
#include <AssetManager.h>

namespace Glory
{
	void ScriptedSystem::OnAdd(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
	}

	void ScriptedSystem::OnStart(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);
		pScript->SetPropertyValues(pObject, pComponent.m_ScriptData);

		pScript->InvokeSafe(pObject, "Start", std::vector<void*>{});
	}

	void ScriptedSystem::OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->InvokeSafe(pObject, "Stop", std::vector<void*>{});
	}

	void ScriptedSystem::OnValidate(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetAssetImmediate<Script>(uuid);
		if (pScript == nullptr) return;

		pScript->LoadScriptProperties(pComponent.m_ScriptProperties, pComponent.m_ScriptData);

		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->SetPropertyValues(pObject, pComponent.m_ScriptData);
		pScript->InvokeSafe(pObject, "OnValidate", std::vector<void*>{});
	}

	void ScriptedSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->InvokeSafe(pObject, "Update", std::vector<void*>{});
		pScript->GetPropertyValues(pObject, pComponent.m_ScriptData);
	}

	void ScriptedSystem::OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->InvokeSafe(pObject, "Draw", std::vector<void*>{});
	}
}
