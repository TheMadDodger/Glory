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
	std::filesystem::path m_DeletingFile;

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
		case T_SceneObject:
		{
			SceneObject* pSceneObject = (SceneObject*)pObject;
			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Type";
			out << YAML::Value << "SceneObject";
			out << YAML::Key << "Value";
			out << YAML::Value << YAML::BeginSeq;
			EditorSceneManager::SerializeObjects(pSceneObject, out);
			out << YAML::EndSeq;
			out << YAML::EndMap;
			ImGui::SetClipboardText(out.c_str());
			break;
		}
		case T_Resource:
		{
			const std::string& path = FileBrowserItem::GetHighlightedPath();
			const UUID uuid = EditorAssetDatabase::FindAssetUUID(path);

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Type";
			out << YAML::Value << "Resource";
			out << YAML::Key << "Value";
			out << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Path";
			out << YAML::Value << path;
			out << YAML::Key << "UUID";
			out << YAML::Value << (uint64_t)uuid;
			out << YAML::EndMap;
			out << YAML::EndMap;
			ImGui::SetClipboardText(out.c_str());

			break;
		}
		case T_Folder:
		{
			const std::string& path = FileBrowserItem::GetHighlightedPath();
			const UUID uuid = EditorAssetDatabase::FindAssetUUID(path);

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Type";
			out << YAML::Value << "Folder";
			out << YAML::Key << "Value";
			out << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Path";
			out << YAML::Value << path;
			out << YAML::EndMap;
			out << YAML::EndMap;
			ImGui::SetClipboardText(out.c_str());
			break;
		}
		}
	}

	OBJECTMENU_CALLBACK(PasteObjectCallback)
	{
		const char* clipboardText = ImGui::GetClipboardText();
		YAML::Node clipboardNode;
		try
		{
			clipboardNode = YAML::Load(clipboardText);
		}
		catch (const std::exception&)
		{
			Debug::LogError("Pasted object is not a YAML object!");
			return;
		}
		if (!clipboardNode.IsDefined() || !clipboardNode.IsMap())
		{
			Debug::LogError("Pasted object is not a YAML object!");
			return;
		}

		YAML::Node typeNode = clipboardNode["Type"];
		YAML::Node valueNode = clipboardNode["Value"];

		if (!typeNode.IsDefined() || !typeNode.IsScalar()) return;
		const std::string& type = typeNode.as<std::string>();

		switch (currentMenu)
		{
		case T_Scene:
		{
			if (type != "SceneObject") return;
			GScene* pScene = (GScene*)pObject;
			EditorSceneManager::PasteSceneObject(pScene, nullptr, valueNode);
			break;
		}
		case T_Hierarchy:
		case T_SceneObject:
		{
			if (type != "SceneObject") return;
			GScene* pScene = EditorSceneManager::GetActiveScene();
			if (!pScene) pScene = EditorSceneManager::NewScene();
			SceneObject* pSceneObject = nullptr;
			if (pObject)
			{
				pSceneObject = (SceneObject*)pObject;
				pScene = pSceneObject->GetScene();
			}
			EditorSceneManager::PasteSceneObject(pScene, pSceneObject, valueNode);
			break;
		}

		case T_Resource:
		case T_Folder:
		case T_ContentBrowser:
		{
			if (type != "Resource" && type != "Folder") return;
			std::filesystem::path sourcePath = valueNode["Path"].as<std::string>();
			std::filesystem::path destinationPath = FileBrowserItem::GetCurrentPath();
			if (type == "Resource")
			{
				std::filesystem::path extension = sourcePath.extension();
				destinationPath.append(sourcePath.filename().replace_extension().string() + " Copy").replace_extension(extension);
				destinationPath = GetUnqiueFilePath(destinationPath);
				std::filesystem::copy(sourcePath, destinationPath);
				EditorAssetDatabase::ImportAssetAsync(destinationPath.string());
			}
			else if (type == "Folder")
			{
				destinationPath.append(sourcePath.filename().string() + " Copy");
				destinationPath = GetUnqiueFilePath(destinationPath);
				std::filesystem::copy(sourcePath, destinationPath);
				EditorAssetDatabase::ImportAssetsAsync(destinationPath.string());
			}
			break;
		}
		}
	}

	OBJECTMENU_CALLBACK(DuplicateObjectCallback)
	{
		switch (currentMenu)
		{
		case T_SceneObject:
		{
			SceneObject* pSceneObject = (SceneObject*)pObject;
			EditorSceneManager::DuplicateSceneObject(pSceneObject);
			break;
		}
		case T_Resource:
		{
			std::filesystem::path path = FileBrowserItem::GetHighlightedPath();
			std::filesystem::path copyPath = path;
			std::filesystem::path extension = copyPath.extension();
			copyPath.replace_filename(copyPath.filename().replace_extension().string() + " Copy").replace_extension(extension);
			copyPath = GetUnqiueFilePath(copyPath);
			std::filesystem::copy(path, copyPath);
			EditorAssetDatabase::ImportAssetAsync(copyPath.string());
			break;
		}
		case T_Folder:
		{
			std::filesystem::path path = FileBrowserItem::GetHighlightedPath();
			std::filesystem::path copyPath = path;
			copyPath.replace_filename(copyPath.filename().string() + " Copy");
			copyPath = GetUnqiueFilePath(copyPath);
			std::filesystem::copy(path, copyPath);
			EditorAssetDatabase::ImportAssetsAsync(copyPath.string());
			break;
		}
		default:
			break;
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
			m_DeletingFile = FileBrowserItem::GetHighlightedPath();
			if (!std::filesystem::exists(m_DeletingFile))
			{
				m_DeletingFile = "";
				return;
			}
			std::vector<std::string> buttons = {
				"No",
				"Yes",
			};
			std::vector<std::function<void()>> callbacks = {
				[]() { m_DeletingFile = ""; PopupManager::CloseCurrentPopup(); },
				[&]() { DeleteResource(m_DeletingFile); m_DeletingFile = ""; PopupManager::CloseCurrentPopup(); },
			};

			std::string name = m_DeletingFile.filename().replace_extension().string();
			PopupManager::OpenModal("Delete " + name, "Are you sure you want to delete \"" + name + "\"?\nThis action cannot be undone!", buttons, callbacks);
			break;
		}

		case ObjectMenuType::T_Folder:
		{
			m_DeletingFile = FileBrowserItem::GetHighlightedPath();
			if (!std::filesystem::exists(m_DeletingFile))
			{
				m_DeletingFile = "";
				return;
			}
			std::vector<std::string> buttons = {
				"No",
				"Yes",
			};
			std::vector<std::function<void()>> callbacks = {
				[]() { m_DeletingFile = ""; PopupManager::CloseCurrentPopup(); },
				[&]() { DeleteFolder(); m_DeletingFile = ""; PopupManager::CloseCurrentPopup(); },
			};
			PopupManager::OpenModal("Delete Folder", "Are you sure you want to delete the folder \"" + m_DeletingFile.filename().string() + "\" and all assets underneath?\nThis action cannot be undone!", buttons, callbacks);
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
		if (!EditorAssetDatabase::AssetExists(uuid)) return;
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
		std::filesystem::path path = m_DeletingFile;
		std::filesystem::path relativePath = path.lexically_relative(Game::GetAssetPath());
		if (!std::filesystem::exists(path)) return;
		if (!std::filesystem::remove_all(path)) return;

		EditorAssetDatabase::DeleteAssets(relativePath.string());
		FileBrowserItem::GetSelectedFolder()->Refresh();
		FileBrowserItem::GetSelectedFolder()->SortChildren();
		PopupManager::CloseCurrentPopup();
	}

	void DeleteResource(std::filesystem::path path)
	{
		if (path.empty()) path = FileBrowserItem::GetHighlightedPath();
		std::filesystem::path relativePath = path.lexically_relative(Game::GetAssetPath());
		if (!std::filesystem::remove(path)) return;

		EditorAssetDatabase::DeleteAsset(relativePath.string());
		FileBrowserItem::GetSelectedFolder()->Refresh();
		FileBrowserItem::GetSelectedFolder()->SortChildren();
	}
}
