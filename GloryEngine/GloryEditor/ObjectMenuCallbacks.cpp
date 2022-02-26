#include "ObjectMenuCallbacks.h"
#include "SceneGraphWindow.h"
#include "Selection.h"
#include "EditorSceneManager.h"
#include <Game.h>
#include <Engine.h>
#include <AssetDatabase.h>

namespace Glory::Editor
{
	OBJECTMENU_CALLBACK(CopyObjectCallback)
	{
		switch (currentMenu)
		{
			
		}
	}

	OBJECTMENU_CALLBACK(PasteObjectCallback)
	{
		switch (currentMenu)
		{

		}
	}

	OBJECTMENU_CALLBACK(DuplicateObjectCallback)
	{
		switch (currentMenu)
		{

		}
	}

	OBJECTMENU_CALLBACK(DeleteObjectCallback)
	{

	}

	OBJECTMENU_CALLBACK(CreateEmptyObjectCallback)
	{
		if (!pObject)
		{
			Selection::SetActiveObject(nullptr);
			GScene* pActiveScene = Game::GetGame().GetEngine()->GetScenesModule()->GetActiveScene();
			if (pActiveScene == nullptr) pActiveScene = EditorSceneManager::NewScene(true);
			SceneObject* pNewObject = pActiveScene->CreateEmptyObject();
			Selection::SetActiveObject(pNewObject);
			return;
		}

		switch (currentMenu)
		{
		case T_Scene:
		{
			Selection::SetActiveObject(nullptr);
			GScene* pScene = (GScene*)pObject;
			if (pScene == nullptr) return;
			SceneObject* pNewObject = pScene->CreateEmptyObject();
			Selection::SetActiveObject(pNewObject);
			break;
		}

		case T_SceneObject:
		{
			Selection::SetActiveObject(nullptr);
			SceneObject* pSceneObject = (SceneObject*)pObject;
			if (pSceneObject == nullptr) return;
			GScene* pScene = pSceneObject->GetScene();
			if (pScene == nullptr) return;
			SceneObject* pNewObject = pScene->CreateEmptyObject();
			pNewObject->SetParent(pSceneObject);
			Selection::SetActiveObject(pNewObject);
			break;
		}
		default:
			break;
		}
	}

	OBJECTMENU_CALLBACK(CreateNewSceneCallback)
	{
		Selection::SetActiveObject(nullptr);
		GScene* pNewScene = EditorSceneManager::NewScene(true);
		Selection::SetActiveObject(pNewScene);
	}

	OBJECTMENU_CALLBACK(SetActiveSceneCallback)
	{
		GScene* pScene = (GScene*)pObject;
		if (pScene == nullptr) return;
		Game::GetGame().GetEngine()->GetScenesModule()->SetActiveScene(pScene);
	}

	OBJECTMENU_CALLBACK(RemoveSceneCallback)
	{
		Selection::SetActiveObject(nullptr);
		GScene* pScene = (GScene*)pObject;
		if (pScene == nullptr) return;
		EditorSceneManager::CloseScene(pScene->GetUUID());
	}

	OBJECTMENU_CALLBACK(ReloadSceneCallback)
	{
		Selection::SetActiveObject(nullptr);
		GScene* pScene = (GScene*)pObject;
		if (pScene == nullptr) return;
		UUID uuid = pScene->GetUUID();
		if (!AssetDatabase::AssetExists(uuid)) return;
		EditorSceneManager::CloseScene(uuid);
		EditorSceneManager::OpenScene(uuid, true);
	}
}