#include "EditorSceneManager.h"
#include "Selection.h"
#include "EditorAssetDatabase.h"
#include "CreateObjectAction.h"
#include "Undo.h"
#include <Game.h>
#include <Engine.h>
#include <ScenesModule.h>
#include <Serializer.h>
#include <TitleBar.h>
#include <tinyfiledialogs.h>

namespace Glory::Editor
{
	std::vector<UUID> EditorSceneManager::m_OpenedSceneIDs;
	std::vector<UUID> EditorSceneManager::m_DirtySceneIDs;
	UUID EditorSceneManager::m_CurrentlySavingScene = 0;

	GScene* EditorSceneManager::NewScene(bool additive)
	{
		if (!additive) CloseAll();
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		GScene* pScene = pScenesModule->CreateEmptyScene();
		m_OpenedSceneIDs.push_back(pScene->GetUUID());
		SetSceneDirty(pScene);
		return pScene;
	}

	void EditorSceneManager::OpenScene(UUID uuid, bool additive)
	{
		if (additive && IsSceneOpen(uuid))
			CloseScene(uuid);
		if (!additive) CloseAll();

		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		AssetLocation location;
		EditorAssetDatabase::GetAssetLocation(uuid, location);
		std::string path = Game::GetGame().GetAssetPath() + "\\" + location.Path;
		pScenesModule->OpenScene(path, uuid);
		m_OpenedSceneIDs.push_back(uuid);

		GScene* pActiveScene = pScenesModule->GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
	}

	void EditorSceneManager::OpenScene(GScene* pScene, UUID uuid)
	{
		if (IsSceneOpen(uuid))
			CloseScene(uuid);

		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		pScenesModule->AddOpenScene(pScene, uuid);
		m_OpenedSceneIDs.push_back(uuid);

		GScene* pActiveScene = pScenesModule->GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
	}

	void EditorSceneManager::SaveOpenScenes()
	{
		std::for_each(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), [](UUID uuid)
		{
			AssetLocation location;
			if (!EditorAssetDatabase::GetAssetLocation(uuid, location)) return; // new scene

			if (location.Path == "") return;
			Save(uuid, Game::GetGame().GetAssetPath() + "\\" + location.Path);
		});
	}

	void EditorSceneManager::CloseScene(UUID uuid)
	{
		// TODO: Check if scene has changes
		Selection::SetActiveObject(nullptr);
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		if (it == m_OpenedSceneIDs.end()) return;
		m_OpenedSceneIDs.erase(it);
		SetSceneDirty(pScenesModule->GetOpenScene(uuid), false);
		pScenesModule->CloseScene(uuid);

		GScene* pActiveScene = pScenesModule->GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
	}

	bool EditorSceneManager::IsSceneOpen(UUID uuid)
	{
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		return it != m_OpenedSceneIDs.end();
	}

	void EditorSceneManager::CloseAll()
	{
		m_OpenedSceneIDs.clear();
		m_DirtySceneIDs.clear();
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		pScenesModule->CloseAllScenes();
		TitleBar::SetText("Scene", "No Scene open");
	}

	size_t EditorSceneManager::OpenSceneCount()
	{
		return m_OpenedSceneIDs.size();
	}

	UUID EditorSceneManager::GetOpenSceneUUID(size_t index)
	{
		return m_OpenedSceneIDs[index];
	}

	GScene* EditorSceneManager::GetOpenScene(size_t index)
	{
		const UUID uuid = GetOpenSceneUUID(index);
		return Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(index);
	}

	GScene* EditorSceneManager::GetOpenScene(UUID uuid)
	{
		return Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(uuid);
	}

	void EditorSceneManager::SaveScene(UUID uuid)
	{
		if (!IsSceneOpen(uuid)) return;
		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(uuid, location))
		{
			SaveSceneAs(uuid);
			return; // new scene
		}

		if (location.Path == "") return;
		Save(uuid, Game::GetGame().GetAssetPath() + "\\" + location.Path);
	}

	void EditorSceneManager::SaveSceneAs(UUID uuid)
	{
		m_CurrentlySavingScene = uuid;

		const char* filters[1] = { "*.gscene" };
		const char* path = tinyfd_saveFileDialog("Save Scene", Game::GetAssetPath().data(), 1, filters, "Glory Scene");

		if (!path) return;

		std::filesystem::path relativePath = path;
		relativePath = relativePath.lexically_relative(Game::GetAssetPath());
		UUID existingAsset = EditorAssetDatabase::FindAssetUUID(relativePath.string());

		if (existingAsset != 0)
		{
			Save(m_CurrentlySavingScene, path);
			CloseScene(m_CurrentlySavingScene);
			OpenScene(existingAsset, true);
			return;
		}

		Save(m_CurrentlySavingScene, path, true);
	}

	void EditorSceneManager::SerializeOpenScenes(YAML::Emitter& out)
	{
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		out << YAML::BeginSeq;
		for (size_t i = 0; i < m_OpenedSceneIDs.size(); i++)
		{
			UUID uuid = m_OpenedSceneIDs[i];
			GScene* pScene = pScenesModule->GetOpenScene(uuid);
			out << YAML::BeginMap;
			out << YAML::Key << "Name";
			out << YAML::Value << pScene->Name();
			out << YAML::Key << "UUID";
			out << YAML::Value << pScene->GetUUID();
			out << YAML::Key << "Scene";
			out << YAML::Value;
			Serializer::SerializeObject(pScene, out);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
	}

	void EditorSceneManager::OpenAllFromNode(YAML::Node& node)
	{
		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node sceneDataNode = node[i];
			YAML::Node nameNode = sceneDataNode["Name"];
			std::string name = nameNode.as<std::string>();
			YAML::Node uuidNode = sceneDataNode["UUID"];
			UUID uuid = uuidNode.as<uint64_t>();
			YAML::Node sceneNode = sceneDataNode["Scene"];
			GScene* pScene = Serializer::DeserializeObjectOfType<GScene>(sceneNode, name);
			OpenScene(pScene, uuid);
		}
	}

	void EditorSceneManager::SetActiveScene(GScene* pScene)
	{
		Game::GetGame().GetEngine()->GetScenesModule()->SetActiveScene(pScene);
		TitleBar::SetText("Scene", pScene ? pScene->Name().c_str() : "No Scene open");
	}

	GScene* EditorSceneManager::GetActiveScene()
	{
		return Game::GetGame().GetEngine()->GetScenesModule()->GetActiveScene();
	}

	void EditorSceneManager::SetSceneDirty(GScene* pScene, bool dirty)
	{
		if (!pScene) return;
		if (!IsSceneOpen(pScene->GetUUID())) return;
		auto itor = std::find(m_DirtySceneIDs.begin(), m_DirtySceneIDs.end(), pScene->GetUUID());
		if (dirty)
		{
			if (itor != m_DirtySceneIDs.end()) return; // Already dirty
			m_DirtySceneIDs.push_back(pScene->GetUUID());
			TitleBar::SetText("SceneChanges", "*");
			return;
		}
		if (itor == m_DirtySceneIDs.end()) return;
		m_DirtySceneIDs.erase(itor);
		TitleBar::SetText("SceneChanges", m_DirtySceneIDs.size() > 0 ? "*" : "");
	}

	bool EditorSceneManager::IsSceneDirty(GScene* pScene)
	{
		return std::find(m_DirtySceneIDs.begin(), m_DirtySceneIDs.end(), pScene->GetUUID()) != m_DirtySceneIDs.end();
	}

	bool EditorSceneManager::HasUnsavedChanges()
	{
		return m_DirtySceneIDs.size() > 0;
	}

	void EditorSceneManager::DuplicateSceneObject(SceneObject* pObject)
	{
		if (!pObject) return;

		/* Get correct parent to deserialize to */
		SceneObject* pParent = pObject->GetParent();
		GScene* pScene = pObject->GetScene();
		if (!pScene) pScene = EditorSceneManager::GetActiveScene();
		if (!pScene) pScene = EditorSceneManager::NewScene();

		/* Serialize the objects entire heirarchy */
		YAML::Emitter out;
		out << YAML::BeginSeq;
		SerializeObjects(pObject, out);
		out << YAML::EndSeq;

		/* Deserialize node into a new objects */
		YAML::Node node = YAML::Load(out.c_str());
		PasteSceneObject(pScene, pParent, node);
	}

	void EditorSceneManager::PasteSceneObject(GScene* pScene, SceneObject* pParent, YAML::Node& node)
	{
		/* Deserialize node into a new objects */
		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node objectNode = node[i];
			if (i == 0 && pParent)
			{
				const UUID parentUUID = pParent->GetUUID();
				objectNode["ParentUUID"] = (uint64_t)parentUUID;
				Serializer::SetUUIDRemap(parentUUID, parentUUID);
			}

			SceneObject* pDupedObject = (SceneObject*)Serializer::DeserializeObject(pScene, objectNode, Serializer::Flags::GenerateNewUUIDs);
			if (i == 0 && pDupedObject)
			{
				Undo::StartRecord("Duplicate", pDupedObject->GetUUID());
				Undo::AddAction(new CreateObjectAction(pDupedObject));
				Undo::StopRecord();
			}
		}

		Serializer::ClearUUIDRemapCache();

		/* Set scene dirty */
		SetSceneDirty(pScene);
	}

	void EditorSceneManager::SerializeObjects(SceneObject* pObject, YAML::Emitter& out)
	{
		Serializer::SerializeObject(pObject, out);
		for (size_t i = 0; i < pObject->ChildCount(); i++)
		{
			SerializeObjects(pObject->GetChild(i), out);
		}
	}

	void EditorSceneManager::Save(UUID uuid, const std::string& path, bool newScene)
	{
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(uuid);
		YAML::Emitter out;
		Serializer::SerializeObject(pScene, out);
		std::ofstream outStream(path);
		outStream << out.c_str();
		outStream.close();
		if (newScene) EditorAssetDatabase::ImportNewScene(path, pScene);
		SetSceneDirty(pScene, false);

		std::stringstream stream;
		stream << "Saved scene to: " << path;
		Debug::LogInfo(stream.str());
	}
}
