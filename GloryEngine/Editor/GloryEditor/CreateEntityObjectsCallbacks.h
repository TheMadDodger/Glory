#pragma once
#include <GloryEditor.h>
#include <ObjectMenu.h>

#define CREATE_OBJECT_CALLBACK_H(name) GLORY_EDITOR_API void Create##name(Object* pObject, const ObjectMenuType& currentMenu);

#define CREATE_OBJECT_CALLBACK_CPP(name, component, ctor) void Create##name(Object* pObject, const ObjectMenuType& currentMenu) \
{\
    SceneObject* pNewObject = CreateNewEmptyObject(pObject, STRINGIFY(name), currentMenu); \
	if (!pNewObject) return; \
	Entity entity = pNewObject->GetEntityHandle(); \
	entity.AddComponent<component>(CTOR_ARGS ctor); \
	Selection::SetActiveObject(nullptr); \
	Undo::StartRecord("Create Empty Object", pNewObject->GetUUID()); \
	Undo::AddAction(new CreateObjectAction(pNewObject)); \
	Undo::StopRecord(); \
	Selection::SetActiveObject(pNewObject); \
}

namespace Glory
{
	class SceneObject;
}

namespace Glory::Editor
{
	GLORY_EDITOR_API SceneObject* CreateNewEmptyObject(Object* pObject, const std::string& name, const ObjectMenuType& currentMenu);

	CREATE_OBJECT_CALLBACK_H(Mesh);
	CREATE_OBJECT_CALLBACK_H(Model);
	CREATE_OBJECT_CALLBACK_H(Camera);
	CREATE_OBJECT_CALLBACK_H(Light);
	CREATE_OBJECT_CALLBACK_H(Scripted);
	CREATE_OBJECT_CALLBACK_H(PhysicsBody);
	CREATE_OBJECT_CALLBACK_H(Character);
}

#define CTOR_ARGS(...) __VA_ARGS__
