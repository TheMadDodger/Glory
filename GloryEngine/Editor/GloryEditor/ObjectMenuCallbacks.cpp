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
#include "EditableEntity.h"
#include "EntityEditor.h"
#include "EditorSceneSerializer.h"
#include "EditorApplication.h"
#include "Package.h"

#include <AssetManager.h>
#include <AssetDatabase.h>
#include <Engine.h>
#include <SceneManager.h>
#include <MaterialInstanceData.h>
#include <PipelineData.h>

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
			EditableEntity* pSceneObject = (EditableEntity*)pObject;
			Utils::InMemoryYAML data;
			auto objects = data.RootNodeRef().ValueRef();
			objects.SetMap();
			objects["Type"].Set("SceneObject");
			auto value = objects["Value"];
			value.SetSequence();
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pSceneObject->SceneID());
			EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, pSceneObject->EntityID(), value);
			ImGui::SetClipboardText(data.ToString().c_str());
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
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

		const char* clipboardText = ImGui::GetClipboardText();
		Utils::InMemoryYAML clipboard{ clipboardText };
		auto clipboardNode = clipboard.RootNodeRef().ValueRef();
		if (clipboard.ParsingFailed())
		{
			pEngine->GetDebug().LogError("Pasted object is not a YAML object!");
			return;
		}

		if (!clipboardNode.Exists() || !clipboardNode.IsMap())
		{
			pEngine->GetDebug().LogError("Pasted object is not a YAML object!");
			return;
		}

		auto typeNode = clipboardNode["Type"];
		auto valueNode = clipboardNode["Value"];

		if (!typeNode.Exists() || !typeNode.IsScalar()) return;
		const std::string& type = typeNode.As<std::string>();

		switch (currentMenu)
		{
		case T_Scene:
		{
			if (type != "SceneObject") return;
			GScene* pScene = (GScene*)pObject;
			EditorApplication::GetInstance()->GetSceneManager().PasteSceneObject(pScene, {}, valueNode);
			break;
		}
		case T_Hierarchy:
		case T_SceneObject:
		{
			if (type != "SceneObject") return;
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetActiveScene();
			if (!pScene) pScene = EditorApplication::GetInstance()->GetSceneManager().NewScene();
			EditableEntity* pSceneObject = nullptr;
			if (pObject)
			{
				pSceneObject = (EditableEntity*)pObject;
				pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pSceneObject->SceneID());
			}
			EditorApplication::GetInstance()->GetSceneManager().PasteSceneObject(pScene, pSceneObject ? pSceneObject->EntityID() : 0, valueNode);
			break;
		}

		case T_Resource:
		case T_Folder:
		case T_ContentBrowser:
		{
			if (type != "Resource" && type != "Folder") return;
			std::filesystem::path sourcePath = valueNode["Path"].As<std::string>();
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
			EditableEntity* pSceneObject = (EditableEntity*)pObject;
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pSceneObject->SceneID());
			Entity entity = pScene->GetEntityByEntityID(pSceneObject->EntityID());
			EditorApplication::GetInstance()->GetSceneManager().DuplicateSceneObject(entity);
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
			EditableEntity* pSceneObject = (EditableEntity*)pObject;
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pSceneObject->SceneID());
			Undo::StartRecord("Delete Object", pSceneObject->GetUUID());
			Undo::AddAction<DeleteSceneObjectAction>(pScene, pSceneObject->EntityID());
			if(Selection::GetActiveObject() == pSceneObject) Selection::SetActiveObjectNoUndo(nullptr);
			DestroyEntity(pSceneObject->EntityID(), pScene);
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
			Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
			GScene* pActiveScene = pEngine->GetSceneManager()->GetActiveScene();
			if (pActiveScene == nullptr) pActiveScene = EditorApplication::GetInstance()->GetSceneManager().NewScene();
			Entity newEnity = pActiveScene->CreateEmptyObject();
			Undo::StartRecord("Create Empty Object", newEnity.EntityUUID());
			Undo::AddAction<CreateObjectAction>(pActiveScene);
			Undo::StopRecord();
			Selection::SetActiveObjectNoUndo(GetEditableEntity(newEnity.GetEntityID(), newEnity.GetScene()));
			return;
		}

		switch (currentMenu)
		{
		case ObjectMenuType::T_Scene:
		{
			Selection::SetActiveObject(nullptr);
			GScene* pScene = (GScene*)pObject;
			if (pScene == nullptr) return;
			Entity newEntity = pScene->CreateEmptyObject();
			Undo::StartRecord("Create Empty Object", newEntity.EntityUUID());
			Undo::AddAction<CreateObjectAction>(pScene);
			Undo::StopRecord();
			Selection::SetActiveObjectNoUndo(GetEditableEntity(newEntity.GetEntityID(), newEntity.GetScene()));
			break;
		}

		case ObjectMenuType::T_SceneObject:
		{
			Selection::SetActiveObject(nullptr);
			EditableEntity* pSceneObject = (EditableEntity*)pObject;
			if (pSceneObject == nullptr) return;
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pSceneObject->SceneID());
			if (pScene == nullptr) return;
			Entity newEntity = pScene->CreateEmptyObject();
			Undo::StartRecord("Create Empty Object", newEntity.EntityUUID());
			Undo::AddAction<CreateObjectAction>(pScene);
			newEntity.SetParent(pSceneObject->EntityID());
			Undo::StopRecord();
			Selection::SetActiveObjectNoUndo(GetEditableEntity(newEntity.GetEntityID(), newEntity.GetScene()));
			break;
		}
		default:
			break;
		}
	}

	OBJECTMENU_CALLBACK(CreateNewSceneCallback)
	{
		Selection::SetActiveObject(nullptr);
		GScene* pNewScene = EditorApplication::GetInstance()->GetSceneManager().NewScene();
		Selection::SetActiveObject(pNewScene);
	}

	OBJECTMENU_CALLBACK(SetActiveSceneCallback)
	{
		GScene* pScene = (GScene*)pObject;
		if (pScene == nullptr) return;
		EditorApplication::GetInstance()->GetSceneManager().SetActiveScene(pScene);
	}

	OBJECTMENU_CALLBACK(RemoveSceneCallback)
	{
		Selection::SetActiveObject(nullptr);
		GScene* pScene = (GScene*)pObject;
		if (pScene == nullptr) return;
		EditorApplication::GetInstance()->GetSceneManager().CloseScene(pScene->GetUUID());

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
		EditorApplication::GetInstance()->GetSceneManager().CloseScene(uuid);
		EditorApplication::GetInstance()->GetSceneManager().OpenScene(uuid, true);
	}

	OBJECTMENU_CALLBACK(CreateNewTextureCallback)
	{
		std::filesystem::path path = FileBrowser::GetCurrentPath();
		path = path.append("NewTexture.gtex");
		path = GetUnqiueFilePath(path);

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gtex");
			if (std::filesystem::exists(finalPath)) return;

			TextureData* pTextureData = new TextureData();
			EditorAssetDatabase::CreateAsset(pTextureData, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	OBJECTMENU_CALLBACK(CreateNewPipelineCallback)
	{
		std::filesystem::path path = FileBrowser::GetCurrentPath();
		path = path.append("NewPipeline.gpln");
		path = GetUnqiueFilePath(path);

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gpln");
			if (std::filesystem::exists(finalPath)) return;

			PipelineData* pPipelineData = new PipelineData();
			EditorAssetDatabase::CreateAsset(pPipelineData, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	OBJECTMENU_CALLBACK(CreateNewMaterialCallback)
	{
		std::filesystem::path path = FileBrowser::GetCurrentPath();
		path = path.append("NewMaterial.gmat");
		path = GetUnqiueFilePath(path);

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gmat");
			if (std::filesystem::exists(finalPath)) return;

			MaterialData* pMaterialData = new MaterialData();
			EditorAssetDatabase::CreateAsset(pMaterialData, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	OBJECTMENU_CALLBACK(CreateNewMaterialInstanceCallback)
	{
		MaterialData* pMaterial = (MaterialData*)pObject;
		if (dynamic_cast<MaterialInstanceData*>(pMaterial)) pMaterial = nullptr;
		std::filesystem::path path = FileBrowser::GetCurrentPath();
		std::string fileName = pMaterial ? pMaterial->Name() + "Instance.gminst" : "NewMaterialInstance.gminst";
		path = path.append(fileName);
		path = GetUnqiueFilePath(path);

		const UUID baseMaterial = pMaterial ? pMaterial->GetUUID() : 0;
		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [baseMaterial](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gminst");
			if (std::filesystem::exists(finalPath)) return;

			MaterialInstanceData* pMaterialData = new MaterialInstanceData(baseMaterial);
			EditorAssetDatabase::CreateAsset(pMaterialData, finalPath.replace_extension("gminst").string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	OBJECTMENU_CALLBACK(CreateNewFolderCallback)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New Folder"));

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "folder", [](std::filesystem::path& finalPath) {
			if (std::filesystem::exists(finalPath)) return;
			if (!std::filesystem::create_directory(finalPath.string())) return;
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
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
		EditorApplication::GetInstance()->GetSceneManager().SaveScene(pScene->GetUUID());
	}

	OBJECTMENU_CALLBACK(SaveSceneAs)
	{
		GScene* pScene = (GScene*)pObject;
		EditorApplication::GetInstance()->GetSceneManager().SaveSceneAs(pScene->GetUUID());
	}

	OBJECTMENU_CALLBACK(ReimportAssetCallback)
	{
		Selection::SetActiveObject(nullptr);
		std::filesystem::path file = FileBrowserItem::GetHighlightedPath();
		const UUID uuid = EditorAssetDatabase::FindAssetUUID(file.string());
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Resource* pLoadedResource = pEngine->GetAssetManager().FindResource(uuid);
		if (pLoadedResource) return;
		EditorAssetDatabase::RemoveAsset(uuid);
		EditorAssetDatabase::ImportAsset(file.string());
	}

	OBJECTMENU_CALLBACK(PackageSceneCallback)
	{
		if (!pObject) return;
		std::filesystem::path file = FileBrowserItem::GetCurrentPath();
		GScene* pScene = static_cast<GScene*>(pObject);
		file.append(pScene->Name()).replace_extension(".gcag");
		PackageScene(pScene, file);
	}

	void DeleteFolder()
	{
		std::filesystem::path path = m_DeletingFile;
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		std::filesystem::path relativePath = path.lexically_relative(pEngine->GetAssetDatabase().GetAssetPath());
		if (!std::filesystem::exists(path)) return;
		if (!std::filesystem::remove_all(path)) return;

		EditorAssetDatabase::DeleteAssets(relativePath.string());
		PopupManager::CloseCurrentPopup();
	}

	void DeleteResource(std::filesystem::path path)
	{
		if (path.empty()) path = FileBrowserItem::GetHighlightedPath();
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		std::filesystem::path relativePath = path.lexically_relative(pEngine->GetAssetDatabase().GetAssetPath());
		if (!std::filesystem::remove(path)) return;

		EditorAssetDatabase::DeleteAsset(relativePath.string());
	}
}
