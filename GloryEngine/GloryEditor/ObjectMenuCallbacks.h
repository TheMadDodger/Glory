#pragma once
#include "ObjectMenu.h"
#include <Object.h>
#include <filesystem>

#define OBJECTMENU_CALLBACK(name) void name(Object* pObject, const ObjectMenuType& currentMenu)

namespace Glory::Editor
{
	std::filesystem::path GetUnqiueFilePath(const std::filesystem::path& start);


	OBJECTMENU_CALLBACK(CopyObjectCallback);
	OBJECTMENU_CALLBACK(PasteObjectCallback);
	OBJECTMENU_CALLBACK(DuplicateObjectCallback);
	OBJECTMENU_CALLBACK(DeleteObjectCallback);
	OBJECTMENU_CALLBACK(CreateEmptyObjectCallback);
	OBJECTMENU_CALLBACK(CreateNewSceneCallback);
	OBJECTMENU_CALLBACK(SetActiveSceneCallback);
	OBJECTMENU_CALLBACK(RemoveSceneCallback);
	OBJECTMENU_CALLBACK(ReloadSceneCallback);
	OBJECTMENU_CALLBACK(CreateNewMaterialCallback);
	OBJECTMENU_CALLBACK(CreateNewMaterialInstanceCallback);
}