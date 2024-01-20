#include "CreateEntityObjectsCallbacks.h"
#include "EditableEntity.h"
#include "EntityEditor.h"
#include "EditorApplication.h"

#include <SceneManager.h>
#include <Engine.h>
#include <EditorSceneManager.h>
#include <CreateObjectAction.h>
#include <Components.h>

namespace Glory::Editor
{
	Entity CreateNewEmptyObject(Object* pObject, const std::string& name, const ObjectMenuType& currentMenu)
    {
		if (!pObject)
		{
			Selection::SetActiveObject(nullptr);
			GScene* pActiveScene = EditorApplication::GetInstance()->GetEngine()->GetSceneManager()->GetActiveScene();
			if (pActiveScene == nullptr) pActiveScene = EditorSceneManager::NewScene(true);
			Entity newEntity = pActiveScene->CreateEmptyObject(name, UUID());
			Selection::SetActiveObject(GetEditableEntity(newEntity.GetEntityID(), newEntity.GetScene()));
			return newEntity;
		}

		switch (currentMenu)
		{
		case ObjectMenuType::T_Scene:
		{
			Selection::SetActiveObject(nullptr);
			GScene* pScene = (GScene*)pObject;
			if (pScene == nullptr) return {};
			Entity newEntity = pScene->CreateEmptyObject(name, UUID());
			Selection::SetActiveObject(GetEditableEntity(newEntity.GetEntityID(), newEntity.GetScene()));
			return newEntity;
		}

		case ObjectMenuType::T_SceneObject:
		{
			Selection::SetActiveObject(nullptr);
			EditableEntity* pSceneObject = (EditableEntity*)pObject;
			if (pSceneObject == nullptr) return {};
			GScene* pScene = EditorSceneManager::GetOpenScene(pSceneObject->SceneID());
			if (pScene == nullptr) return {};
			Entity newEntity = pScene->CreateEmptyObject(name, UUID());
			newEntity.SetParent(pSceneObject->EntityID());
			Selection::SetActiveObject(GetEditableEntity(newEntity.GetEntityID(), newEntity.GetScene()));
			return newEntity;
		}
		}

		return {};
    }

	CREATE_OBJECT_CALLBACK_CPP(Mesh, MeshRenderer, ());
	CREATE_OBJECT_CALLBACK_CPP(Model, ModelRenderer, ());
	CREATE_OBJECT_CALLBACK_CPP(Camera, CameraComponent, ());
	CREATE_OBJECT_CALLBACK_CPP(Light, LightComponent, ());
}
