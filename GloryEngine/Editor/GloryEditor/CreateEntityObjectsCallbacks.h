#pragma once
#include <GloryEditor.h>
#include <ObjectMenu.h>
#include <Entity.h>

#define CREATE_OBJECT_CALLBACK_H(name) GLORY_EDITOR_API void Create##name(Object* pObject, const ObjectMenuType& currentMenu);

#define CREATE_OBJECT_CALLBACK_CPP(name, component, ctor) void Create##name(Object* pObject, const ObjectMenuType& currentMenu) \
{\
    Entity newEntity = CreateNewEmptyObject(pObject, STRINGIFY(name), currentMenu); \
	Undo::StartRecord("Create Empty Object", newEntity.EntityUUID()); \
	component& comp = newEntity.AddComponent<component>(CTOR_ARGS ctor); \
	Undo::AddAction<CreateObjectAction>(newEntity.GetScene()); \
	Selection::SetActiveObject(GetEditableEntity(newEntity.GetEntityID(), newEntity.GetScene())); \
	Undo::StopRecord(); \
	auto registry = newEntity.GetRegistry(); \
	auto pTypeView = newEntity.GetRegistry()->GetTypeView<component>(); \
	if (newEntity.IsActive()) \
	{ \
		pTypeView->Invoke(Utils::ECS::InvocationType::OnEnable, registry, newEntity.GetEntityID(), &comp); \
		pTypeView->Invoke(Utils::ECS::InvocationType::OnEnableDraw, registry, newEntity.GetEntityID(), &comp); \
	} \
}

namespace Glory::Editor
{
	GLORY_EDITOR_API Entity CreateNewEmptyObject(Object* pObject, const std::string& name, const ObjectMenuType& currentMenu);

	CREATE_OBJECT_CALLBACK_H(Mesh);
	CREATE_OBJECT_CALLBACK_H(Model);
	CREATE_OBJECT_CALLBACK_H(Camera);
	CREATE_OBJECT_CALLBACK_H(Light);
	CREATE_OBJECT_CALLBACK_H(PhysicsBody);
	CREATE_OBJECT_CALLBACK_H(Character);
	CREATE_OBJECT_CALLBACK_H(Text);
}

#define CTOR_ARGS(...) __VA_ARGS__
