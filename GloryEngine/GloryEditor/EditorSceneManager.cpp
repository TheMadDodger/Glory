#include "EditorSceneManager.h"
#include "FileDialog.h"
#include "Selection.h"
#include <Game.h>
#include <Engine.h>
#include <ScenesModule.h>
#include <AssetDatabase.h>
#include <Serializer.h>

namespace Glory::Editor
{
	std::vector<UUID> EditorSceneManager::m_OpenedSceneIDs;
	UUID EditorSceneManager::m_CurrentlySavingScene = 0;

	GLORY_EDITOR_API GScene* EditorSceneManager::NewScene(bool additive)
	{
		if (!additive) CloseAll();
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		GScene* pScene = pScenesModule->CreateEmptyScene();
		m_OpenedSceneIDs.push_back(pScene->GetUUID());
		return pScene;
	}

	GLORY_EDITOR_API void EditorSceneManager::OpenScene(UUID uuid, bool additive)
	{
		if (additive && IsSceneOpen(uuid))
			CloseScene(uuid);
		if (!additive) CloseAll();

		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		AssetLocation location;
		AssetDatabase::GetAssetLocation(uuid, location);
		std::string path = Game::GetGame().GetAssetPath() + "\\" + location.m_Path;
		pScenesModule->OpenScene(path, uuid);
		m_OpenedSceneIDs.push_back(uuid);
	}

	GLORY_EDITOR_API void EditorSceneManager::OpenScene(GScene* pScene, UUID uuid)
	{
		if (IsSceneOpen(uuid))
			CloseScene(uuid);

		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		pScenesModule->AddOpenScene(pScene, uuid);
		m_OpenedSceneIDs.push_back(uuid);
	}

	GLORY_EDITOR_API void EditorSceneManager::SaveOpenScenes()
	{
		std::for_each(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), [](UUID uuid)
		{
			AssetLocation location;
			if (!AssetDatabase::GetAssetLocation(uuid, location)) return; // new scene

			if (location.m_Path == "") return;
			Save(uuid, Game::GetGame().GetAssetPath() + "\\" + location.m_Path);
		});
	}

	GLORY_EDITOR_API void EditorSceneManager::CloseScene(UUID uuid)
	{
		// TODO: Check if scene has changes
		Selection::SetActiveObject(nullptr);
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		pScenesModule->CloseScene(uuid);
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		if (it == m_OpenedSceneIDs.end()) return;
		m_OpenedSceneIDs.erase(it);
	}

	GLORY_EDITOR_API bool EditorSceneManager::IsSceneOpen(UUID uuid)
	{
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		return it != m_OpenedSceneIDs.end();
	}

	GLORY_EDITOR_API void EditorSceneManager::CloseAll()
	{
		m_OpenedSceneIDs.clear();
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		pScenesModule->CloseAllScenes();
	}

	GLORY_EDITOR_API size_t EditorSceneManager::OpenSceneCount()
	{
		return m_OpenedSceneIDs.size();
	}

	GLORY_EDITOR_API UUID EditorSceneManager::GetOpenSceneUUID(size_t index)
	{
		return m_OpenedSceneIDs[index];
	}

	GLORY_EDITOR_API void EditorSceneManager::SaveScene(UUID uuid)
	{
		if (!IsSceneOpen(uuid)) return;
		AssetLocation location;
		if (!AssetDatabase::GetAssetLocation(uuid, location))
		{
			SaveSceneAs(uuid);
			return; // new scene
		}

		if (location.m_Path == "") return;
		Save(uuid, Game::GetGame().GetAssetPath() + "\\" + location.m_Path);
	}

	GLORY_EDITOR_API void EditorSceneManager::SaveSceneAs(UUID uuid)
	{
		m_CurrentlySavingScene = uuid;
		FileDialog::Save("SceneSaveDialog", "Save scene", "Glory Scene (*.gscene){.gscene}", Game::GetAssetPath(), [&](const std::string& result)
		{
			if (result == "") return;

			std::filesystem::path relativePath = result;
			relativePath = relativePath.lexically_relative(Game::GetAssetPath());
			UUID existingAsset = AssetDatabase::GetAssetUUID(relativePath.string());

			if (existingAsset != 0)
			{
				Save(m_CurrentlySavingScene, result);
				CloseScene(m_CurrentlySavingScene);
				OpenScene(existingAsset, true);
				return;
			}

			Save(m_CurrentlySavingScene, result, true);
		});
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

	GLORY_EDITOR_API void EditorSceneManager::OpenAllFromNode(YAML::Node& node)
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

	void EditorSceneManager::Save(UUID uuid, const std::string& path, bool newScene)
	{
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(uuid);
		YAML::Emitter out;
		Serializer::SerializeObject(pScene, out);
		std::ofstream outStream(path);
		outStream << out.c_str();
		outStream.close();

		if (newScene)
		{
			AssetDatabase::ImportNewScene(path, pScene);
			UUID newUUID = pScene->GetUUID();

			//std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
			//std::replace(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid, newUUID);
			pScene->SetName(AssetDatabase::GetAssetName(newUUID));
			CloseScene(uuid);
			CloseScene(newUUID);
			OpenScene(newUUID, true);
		}
	}
}
