#pragma once
#include "ObjectMenu.h"
#include "GloryEditor.h"
#include <Object.h>
#include <filesystem>

#define OBJECTMENU_CALLBACK(name) GLORY_EDITOR_API void name(Object* pObject, const ObjectMenuType& currentMenu)

namespace Glory::Editor
{
	GLORY_EDITOR_API std::filesystem::path GetUnqiueFilePath(const std::filesystem::path& start);

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
	OBJECTMENU_CALLBACK(CreateNewFolderCallback);
	OBJECTMENU_CALLBACK(RenameItemCallback);
	OBJECTMENU_CALLBACK(SaveScene);
	OBJECTMENU_CALLBACK(SaveSceneAs);
	OBJECTMENU_CALLBACK(ReimportAssetCallback);

	GLORY_EDITOR_API void DeleteFolder();
	GLORY_EDITOR_API void DeleteResource();
}