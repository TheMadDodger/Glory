#include "ScriptedSystem.h"
#include "EntityScene.h"
#include <AssetReferencePropertyTemplate.h>
#include <SerializedPropertyManager.h>
#include <AssetManager.h>

namespace Glory
{
	void ScriptedSystem::OnStart(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Start()", nullptr);
	}

	void ScriptedSystem::OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Stop()", nullptr);
	}

	void ScriptedSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Update()", nullptr);
	}

	void ScriptedSystem::OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		UUID uuid = pComponent.m_Script.AssetUUID();
		if (!uuid) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(uuid);
		if (pScript == nullptr) return;
		ScenesModule* pEntityScenes = Game::GetGame().GetEngine()->GetScenesModule();
		SceneObject* pObject = pEntityScenes->GetSceneObjectFromObjectID(entity);
		pScript->Invoke(pObject, "Draw()", nullptr);
	}
}
