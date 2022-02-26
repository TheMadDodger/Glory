#pragma once
#include "ObjectMenu.h"
#include <Object.h>

#define OBJECTMENU_CALLBACK(name) void name(Object* pObject, const ObjectMenuType& currentMenu)

namespace Glory::Editor
{
	OBJECTMENU_CALLBACK(CopyObjectCallback);
	OBJECTMENU_CALLBACK(PasteObjectCallback);
	OBJECTMENU_CALLBACK(DuplicateObjectCallback);
	OBJECTMENU_CALLBACK(DeleteObjectCallback);
	OBJECTMENU_CALLBACK(CreateEmptyObjectCallback);
	OBJECTMENU_CALLBACK(CreateNewSceneCallback);
	OBJECTMENU_CALLBACK(SetActiveSceneCallback);
	OBJECTMENU_CALLBACK(RemoveSceneCallback);
	OBJECTMENU_CALLBACK(ReloadSceneCallback);
}