#include "CreateEntityObjectsCallbacks.h"

#include <SceneManager.h>
#include <SceneObject.h>
#include <Game.h>
#include <Engine.h>
#include <EditorSceneManager.h>
#include <CreateObjectAction.h>
#include <Components.h>

namespace Glory::Editor
{
	SceneObject* CreateNewEmptyObject(Object* pObject, const std::string& name, const ObjectMenuType& currentMenu)
    {
		if (!pObject)
		{
			Selection::SetActiveObject(nullptr);
			GScene* pActiveScene = Game::GetGame().GetEngine()->GetSceneManager()->GetActiveScene();
			if (pActiveScene == nullptr) pActiveScene = EditorSceneManager::NewScene(true);
			SceneObject* pNewObject = pActiveScene->CreateEmptyObject(name, UUID());
			Selection::SetActiveObject(pNewObject);
			return (SceneObject*)pNewObject;
		}

		switch (currentMenu)
		{
		case ObjectMenuType::T_Scene:
		{
			Selection::SetActiveObject(nullptr);
			GScene* pScene = (GScene*)pObject;
			if (pScene == nullptr) return nullptr;
			SceneObject* pNewObject = pScene->CreateEmptyObject(name, UUID());
			Selection::SetActiveObject(pNewObject);
			return (SceneObject*)pNewObject;
		}

		case ObjectMenuType::T_SceneObject:
		{
			Selection::SetActiveObject(nullptr);
			SceneObject* pSceneObject = (SceneObject*)pObject;
			if (pSceneObject == nullptr) return nullptr;
			GScene* pScene = pSceneObject->GetScene();
			if (pScene == nullptr) return nullptr;
			SceneObject* pNewObject = pScene->CreateEmptyObject(name, UUID());
			pNewObject->SetParent(pSceneObject);
			Selection::SetActiveObject(pNewObject);
			return (SceneObject*)pNewObject;
		}
		}

		return nullptr;
    }

	CREATE_OBJECT_CALLBACK_CPP(Mesh, MeshRenderer, ());
	CREATE_OBJECT_CALLBACK_CPP(Model, ModelRenderer, ());
	CREATE_OBJECT_CALLBACK_CPP(Camera, CameraComponent, ());
	CREATE_OBJECT_CALLBACK_CPP(Light, LightComponent, ());
	CREATE_OBJECT_CALLBACK_CPP(Scripted, ScriptedComponent, ());
}
