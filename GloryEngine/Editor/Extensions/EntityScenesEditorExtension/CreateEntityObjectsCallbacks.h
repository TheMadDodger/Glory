#pragma once
#include <GloryEditor.h>
#include <ObjectMenu.h>

#define CREATE_OBJECT_CALLBACK_H(name) GLORY_EDITOR_API void Create##name(Object* pObject, const ObjectMenuType& currentMenu);

namespace Glory::Editor
{
	CREATE_OBJECT_CALLBACK_H(Mesh);
	CREATE_OBJECT_CALLBACK_H(Model);
	CREATE_OBJECT_CALLBACK_H(Camera);
	CREATE_OBJECT_CALLBACK_H(Light);
	CREATE_OBJECT_CALLBACK_H(Scripted);
	CREATE_OBJECT_CALLBACK_H(PhysicsBody);
	CREATE_OBJECT_CALLBACK_H(Character);
}
