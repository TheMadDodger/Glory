#include "ScriptedSystem.h"
#include "EntityScene.h"
#include <AssetReferencePropertyTemplate.h>
#include <SerializedPropertyManager.h>
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

		pScript->Invoke(pObject, "Start()", nullptr);
	}

	void ScriptedSystem::OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Stop()", nullptr);
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
		pScript->Invoke(pObject, "OnValidate()", nullptr);
	}

	void ScriptedSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		const UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Update()", nullptr);
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
		pScript->Invoke(pObject, "Draw()", nullptr);
	}
}
