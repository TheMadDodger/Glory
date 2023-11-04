#include "CreateEntityObjectsCallbacks.h"

#include <ScenesModule.h>
#include <EntitySceneObject.h>
#include <Game.h>
#include <Engine.h>
#include <EditorSceneManager.h>
#include <CreateObjectAction.h>
#include <Components.h>

#define CTOR_ARGS(...) __VA_ARGS__

#define CREATE_OBJECT_CALLBACK_CPP(name, component, ctor) void Create##name(Object* pObject, const ObjectMenuType& currentMenu) \
{\
    EntitySceneObject* pNewObject = CreateNewEmptyObject(pObject, STRINGIFY(name), currentMenu); \
	if (!pNewObject) return; \
	Entity entity = pNewObject->GetEntityHandle(); \
	entity.AddComponent<component>(CTOR_ARGS ctor); \
	Selection::SetActiveObject(nullptr); \
	Undo::StartRecord("Create Empty Object", pNewObject->GetUUID()); \
	Undo::AddAction(new CreateObjectAction(pNewObject)); \
	Undo::StopRecord(); \
	Selection::SetActiveObject(pNewObject); \
}

namespace Glory::Editor
{
	EntitySceneObject* CreateNewEmptyObject(Object* pObject, const std::string& name, const ObjectMenuType& currentMenu)
    {
		if (!pObject)
		{
			Selection::SetActiveObject(nullptr);
			GScene* pActiveScene = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetActiveScene();
			if (pActiveScene == nullptr) pActiveScene = EditorSceneManager::NewScene(true);
			SceneObject* pNewObject = pActiveScene->CreateEmptyObject(name, UUID());
			Selection::SetActiveObject(pNewObject);
			return (EntitySceneObject*)pNewObject;
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
			return (EntitySceneObject*)pNewObject;
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
			return (EntitySceneObject*)pNewObject;
		}
		}

		return nullptr;
    }

	CREATE_OBJECT_CALLBACK_CPP(Mesh, MeshRenderer, ());
	CREATE_OBJECT_CALLBACK_CPP(Model, ModelRenderer, ());
	CREATE_OBJECT_CALLBACK_CPP(Camera, CameraComponent, ());
	CREATE_OBJECT_CALLBACK_CPP(Light, LightComponent, ());
	CREATE_OBJECT_CALLBACK_CPP(Scripted, ScriptedComponent, ());
	CREATE_OBJECT_CALLBACK_CPP(PhysicsBody, PhysicsBody, ());
	CREATE_OBJECT_CALLBACK_CPP(Character, CharacterController, ());
}
