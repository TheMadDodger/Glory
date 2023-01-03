#include "ObjectMenuCallbacks.h"
#include "SceneGraphWindow.h"
#include "Selection.h"
#include "EditorSceneManager.h"
#include "PopupManager.h"
#include "Undo.h"
#include "FileBrowser.h"
#include "CreateObjectAction.h"
#include "DeleteSceneObjectAction.h"
#include "EditorAssetDatabase.h"
#include <AssetManager.h>
#include <Game.h>
#include <Engine.h>
#include <MaterialInstanceData.h>

namespace Glory::Editor
{
	std::filesystem::path GetUnqiueFilePath(const std::filesystem::path& start)
	{
		std::filesystem::path currentPath = start;
		std::filesystem::path extenstion = start.extension();
		std::filesystem::path fileName = currentPath.filename();
		fileName = fileName.replace_extension("");
		size_t counter = 0;
		while (std::filesystem::exists(currentPath))
		{
			++counter;
			currentPath.replace_filename(fileName.string() + std::to_string(counter));
			currentPath.replace_extension(extenstion);
		}

		return currentPath;
	}

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
		switch (currentMenu)
		{
		case ObjectMenuType::T_SceneObject:
		{
			SceneObject* pSceneObject = (SceneObject*)pObject;
			GScene* pScene = pSceneObject->GetScene();
			if(Selection::GetActiveObject() == pSceneObject) Selection::SetActiveObject(nullptr);
			Undo::StartRecord("Delete Object", pSceneObject->GetUUID());
			Undo::AddAction(new DeleteSceneObjectAction(pSceneObject));
			pScene->DeleteObject(pSceneObject);
			Undo::StopRecord();
			break;
		}

		case ObjectMenuType::T_Resource:
		{
			std::vector<std::string> buttons = {
				"No",
				"Yes",
			};
			std::vector<std::function<void()>> callbacks = {
				[]() { PopupManager::CloseCurrentPopup(); },
				[&]() { DeleteResource(); },
			};

			std::filesystem::path file = FileBrowserItem::GetHighlightedPath();
			std::string name = file.filename().replace_extension().string();
			PopupManager::OpenModal("Delete " + name, "Are you sure you want to delete \"" + name + "\"?\nThis action cannot be undone!", buttons, callbacks);
			break;
		}

		case ObjectMenuType::T_Folder:
		{
			std::filesystem::path path = FileBrowserItem::GetHighlightedPath();
			std::vector<std::string> buttons = {
				"No",
				"Yes",
			};
			std::vector<std::function<void()>> callbacks = {
				[]() { PopupManager::CloseCurrentPopup(); },
				[&]() { DeleteFolder(); },
			};
			PopupManager::OpenModal("Delete Folder", "Are you sure you want to delete the folder \"" + path.filename().string() + "\" and all assets underneath?\nThis action cannot be undone!", buttons, callbacks);
			break;
		}

		case ObjectMenuType::T_Scene:
		{
			RemoveSceneCallback(pObject, currentMenu);
			break;
		}

		}
	}

	OBJECTMENU_CALLBACK(CreateEmptyObjectCallback)
	{
		if (!pObject)
		{
			Selection::SetActiveObject(nullptr);
			GScene* pActiveScene = Game::GetGame().GetEngine()->GetScenesModule()->GetActiveScene();
			if (pActiveScene == nullptr) pActiveScene = EditorSceneManager::NewScene(true);
			SceneObject* pNewObject = pActiveScene->CreateEmptyObject();
			Undo::StartRecord("Create Empty Object", pNewObject->GetUUID());
			Undo::AddAction(new CreateObjectAction(pNewObject));
			Undo::StopRecord();
			Selection::SetActiveObject(pNewObject);
			return;
		}

		switch (currentMenu)
		{
		case ObjectMenuType::T_Scene:
		{
			Selection::SetActiveObject(nullptr);
			GScene* pScene = (GScene*)pObject;
			if (pScene == nullptr) return;
			SceneObject* pNewObject = pScene->CreateEmptyObject();
			Undo::StartRecord("Create Empty Object", pNewObject->GetUUID());
			Undo::AddAction(new CreateObjectAction(pNewObject));
			Undo::StopRecord();
			Selection::SetActiveObject(pNewObject);
			break;
		}

		case ObjectMenuType::T_SceneObject:
		{
			Selection::SetActiveObject(nullptr);
			SceneObject* pSceneObject = (SceneObject*)pObject;
			if (pSceneObject == nullptr) return;
			GScene* pScene = pSceneObject->GetScene();
			if (pScene == nullptr) return;
			SceneObject* pNewObject = pScene->CreateEmptyObject();
			Undo::StartRecord("Create Empty Object", pNewObject->GetUUID());
			Undo::AddAction(new CreateObjectAction(pNewObject));
			pNewObject->SetParent(pSceneObject);
			Undo::StopRecord();
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
		EditorSceneManager::SetActiveScene(pScene);
	}

	OBJECTMENU_CALLBACK(RemoveSceneCallback)
	{
		Selection::SetActiveObject(nullptr);
		GScene* pScene = (GScene*)pObject;
		if (pScene == nullptr) return;
		EditorSceneManager::CloseScene(pScene->GetUUID());

		/* TODO: To prevent crashing the Undo history must be cleared :/
		 * Need to add a RemoveSceneAction and serialize the whole scene in it
		 * then bring it back on undo. */
		Undo::Clear();
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

	OBJECTMENU_CALLBACK(CreateNewMaterialCallback)
	{
		std::filesystem::path path = FileBrowser::GetCurrentPath();
		path = path.append("NewMaterial.gmat");
		path = GetUnqiueFilePath(path);
		MaterialData* pMaterialData = new MaterialData();
		EditorAssetDatabase::CreateAsset(pMaterialData, path.string());

		FileBrowserItem::GetSelectedFolder()->Refresh();
		FileBrowserItem::GetSelectedFolder()->SortChildren();
		FileBrowser::BeginRename(path.filename().string(), false);
	}

	OBJECTMENU_CALLBACK(CreateNewMaterialInstanceCallback)
	{
		MaterialData* pMaterial = (MaterialData*)pObject;
		if (dynamic_cast<MaterialInstanceData*>(pMaterial)) pMaterial = nullptr;
		std::filesystem::path path = FileBrowser::GetCurrentPath();
		std::string fileName = pMaterial ? pMaterial->Name() + "Instance.gminst" : "NewMaterialInstance.gminst";
		path = path.append(fileName);
		path = GetUnqiueFilePath(path);
		MaterialInstanceData* pMaterialData = new MaterialInstanceData(pMaterial);
		EditorAssetDatabase::CreateAsset(pMaterialData, path.string());

		FileBrowserItem::GetSelectedFolder()->Refresh();
		FileBrowserItem::GetSelectedFolder()->SortChildren();
		FileBrowser::BeginRename(fileName, false);
	}

	OBJECTMENU_CALLBACK(CreateNewFolderCallback)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New Folder"));
		if (!std::filesystem::create_directory(path)) return;
		path.filename();

		FileBrowserItem::GetSelectedFolder()->Refresh();
		FileBrowserItem::GetSelectedFolder()->SortChildren();
		FileBrowser::BeginRename(path.filename().string(), true);
	}

	OBJECTMENU_CALLBACK(RenameItemCallback)
	{
		std::string itemToRename = "";
		std::filesystem::path path = FileBrowserItem::GetHighlightedPath();
		itemToRename = path.filename().replace_extension("").string();
		FileBrowser::BeginRename(itemToRename, currentMenu == ObjectMenuType::T_Folder);
	}

	OBJECTMENU_CALLBACK(SaveScene)
	{
		GScene* pScene = (GScene*)pObject;
		EditorSceneManager::SaveScene(pScene->GetUUID());
	}

	OBJECTMENU_CALLBACK(SaveSceneAs)
	{
		GScene* pScene = (GScene*)pObject;
		EditorSceneManager::SaveSceneAs(pScene->GetUUID());
	}

	OBJECTMENU_CALLBACK(ReimportAssetCallback)
	{
		Selection::SetActiveObject(nullptr);
		std::filesystem::path file = FileBrowserItem::GetHighlightedPath();
		UUID uuid = EditorAssetDatabase::FindAssetUUID(file.string());
		Resource* pLoadedResource = AssetManager::FindResource(uuid);
		if (pLoadedResource) return;
		EditorAssetDatabase::RemoveAsset(uuid);
		EditorAssetDatabase::ImportAsset(file.string(), pLoadedResource);
	}

	void DeleteFolder()
	{
		std::filesystem::path path = FileBrowserItem::GetHighlightedPath();
		std::filesystem::path relativePath = path.lexically_relative(Game::GetAssetPath());
		if (!std::filesystem::remove_all(path)) return;

		EditorAssetDatabase::DeleteAssets(relativePath.string());
		FileBrowserItem::GetSelectedFolder()->Refresh();
		FileBrowserItem::GetSelectedFolder()->SortChildren();
		PopupManager::CloseCurrentPopup();
	}

	void DeleteResource()
	{
		std::filesystem::path path = FileBrowserItem::GetHighlightedPath();
		std::filesystem::path relativePath = path.lexically_relative(Game::GetAssetPath());
		if (!std::filesystem::remove(path)) return;

		EditorAssetDatabase::DeleteAsset(relativePath.string());
		FileBrowserItem::GetSelectedFolder()->Refresh();
		FileBrowserItem::GetSelectedFolder()->SortChildren();
		PopupManager::CloseCurrentPopup();
	}
}
