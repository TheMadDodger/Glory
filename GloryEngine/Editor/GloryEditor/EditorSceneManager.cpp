#include "EditorSceneManager.h"
#include "Selection.h"
#include "EditorAssetDatabase.h"
#include "EditorSceneSerializer.h"
#include "CreateObjectAction.h"
#include "Undo.h"
#include "EditorApplication.h"

#include <Engine.h>
#include <AssetDatabase.h>
#include <SceneManager.h>
#include <TitleBar.h>
#include <tinyfiledialogs.h>
#include <ProjectSpace.h>

namespace Glory::Editor
{
	std::vector<UUID> EditorSceneManager::m_OpenedSceneIDs;
	std::vector<UUID> EditorSceneManager::m_DirtySceneIDs;
	UUID EditorSceneManager::m_CurrentlySavingScene = 0;

	GScene* EditorSceneManager::NewScene(bool additive)
	{
		if (!additive) CloseAll();
		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		GScene* pScene = pScenes->CreateEmptyScene();
		m_OpenedSceneIDs.push_back(pScene->GetUUID());
		SetSceneDirty(pScene);
		return pScene;
	}

	void EditorSceneManager::OpenScene(UUID uuid, bool additive)
	{
		if (additive && IsSceneOpen(uuid))
			CloseScene(uuid);
		if (!additive) CloseAll();

		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		AssetLocation location;
		EditorAssetDatabase::GetAssetLocation(uuid, location);
		std::string path = std::string{ EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath() } + "\\" + location.Path;

		YAML::Node node = YAML::LoadFile(path);
		std::filesystem::path filePath = path;
		GScene* pScene = EditorSceneSerializer::DeserializeScene(EditorApplication::GetInstance()->GetEngine(), node, uuid, filePath.filename().replace_extension().string());
		if (pScene == nullptr) return;

		pScenes->AddOpenScene(pScene);
		m_OpenedSceneIDs.push_back(uuid);

		GScene* pActiveScene = pScenes->GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
	}

	void EditorSceneManager::OpenScene(GScene* pScene, UUID uuid)
	{
		if (IsSceneOpen(uuid))
			CloseScene(uuid);

		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		pScenes->AddOpenScene(pScene, uuid);
		m_OpenedSceneIDs.push_back(uuid);

		GScene* pActiveScene = pScenes->GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
	}

	void EditorSceneManager::SaveOpenScenes()
	{
		std::for_each(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), [](UUID uuid)
		{
			AssetLocation location;
			if (!EditorAssetDatabase::GetAssetLocation(uuid, location)) return; // new scene

			if (location.Path == "") return;
			Save(uuid, std::string{ EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath() } + "\\" + location.Path);
		});
	}

	void EditorSceneManager::CloseScene(UUID uuid)
	{
		// TODO: Check if scene has changes
		Selection::SetActiveObject(nullptr);
		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		if (it == m_OpenedSceneIDs.end()) return;
		m_OpenedSceneIDs.erase(it);
		SetSceneDirty(pScenes->GetOpenScene(uuid), false);
		pScenes->CloseScene(uuid);

		GScene* pActiveScene = pScenes->GetActiveScene();
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
		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		pScenes->CloseAllScenes();
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
		return EditorApplication::GetInstance()->GetEngine()->GetSceneManager()->GetOpenScene(index);
	}

	GScene* EditorSceneManager::GetOpenScene(UUID uuid)
	{
		return EditorApplication::GetInstance()->GetEngine()->GetSceneManager()->GetOpenScene(uuid);
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
		Save(uuid, std::string{ EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath() } + "\\" + location.Path);
	}

	void EditorSceneManager::SaveSceneAs(UUID uuid)
	{
		m_CurrentlySavingScene = uuid;

		const char* filters[1] = { "*.gscene" };
		const char* path = tinyfd_saveFileDialog("Save Scene", EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath().data(), 1, filters, "Glory Scene");

		if (!path) return;

		std::filesystem::path relativePath = path;
		relativePath = relativePath.lexically_relative(EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath());
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
		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		out << YAML::BeginSeq;
		for (size_t i = 0; i < m_OpenedSceneIDs.size(); i++)
		{
			UUID uuid = m_OpenedSceneIDs[i];
			GScene* pScene = pScenes->GetOpenScene(uuid);
			out << YAML::BeginMap;
			out << YAML::Key << "Name";
			out << YAML::Value << pScene->Name();
			out << YAML::Key << "UUID";
			out << YAML::Value << pScene->GetUUID();
			out << YAML::Key << "Scene";
			out << YAML::Value << YAML::BeginMap;
			EditorSceneSerializer::SerializeScene(EditorApplication::GetInstance()->GetEngine(), pScene, out);
			out << YAML::EndMap;
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
			GScene* pScene = EditorSceneSerializer::DeserializeScene(EditorApplication::GetInstance()->GetEngine(), sceneNode, uuid, name);
			OpenScene(pScene, uuid);
		}
	}

	void EditorSceneManager::SetActiveScene(GScene* pScene)
	{
		EditorApplication::GetInstance()->GetEngine()->GetSceneManager()->SetActiveScene(pScene);
		TitleBar::SetText("Scene", pScene ? pScene->Name().c_str() : "No Scene open");
	}

	GScene* EditorSceneManager::GetActiveScene()
	{
		GScene* pScene = EditorApplication::GetInstance()->GetEngine()->GetSceneManager()->GetActiveScene();
		if (!pScene) pScene = NewScene();
		return pScene;
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

	void EditorSceneManager::DuplicateSceneObject(Entity entity)
	{
		if (!entity.IsValid()) return;

		/* Get correct parent to deserialize to */
		GScene* pScene = entity.GetScene();
		if (!pScene) pScene = EditorSceneManager::GetActiveScene();
		if (!pScene) pScene = EditorSceneManager::NewScene();

		/* Serialize the objects entire heirarchy */
		YAML::Emitter out;
		out << YAML::BeginSeq;
		EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, entity.GetEntityID(), out);
		out << YAML::EndSeq;

		/* Deserialize node into a new objects */
		YAML::Node node = YAML::Load(out.c_str());
		PasteSceneObject(pScene, entity.Parent(), node);
	}

	void EditorSceneManager::PasteSceneObject(GScene* pScene, Utils::ECS::EntityID parent, YAML::Node& node)
	{
		/* Deserialize node into a new objects */
		Entity parentEntity = pScene->GetEntityByEntityID(parent);
		Utils::NodeRef entities{node};
		for (size_t i = 0; i < entities.ValueRef().Size(); i++)
		{
			Utils::NodeValueRef entity = entities.ValueRef()[i];
			if (i == 0 && parentEntity.IsValid())
			{
				const UUID parentUUID = parentEntity.EntityUUID();
				entity["ParentUUID"].Set((uint64_t)parentUUID);
				EditorApplication::GetInstance()->GetEngine()->m_UUIDRemapper.EnforceRemap(parentUUID, parentUUID);
			}

			Entity newEntity = EditorSceneSerializer::DeserializeEntity(EditorApplication::GetInstance()->GetEngine(), pScene, entity.Node(), EditorSceneSerializer::Flags::GenerateNewUUIDs);
			if (i == 0 && newEntity.IsValid())
			{
				Undo::StartRecord("Duplicate", newEntity.EntityUUID());
				Undo::AddAction(new CreateObjectAction(pScene));
				Undo::StopRecord();
			}
		}

		EditorApplication::GetInstance()->GetEngine()->m_UUIDRemapper.Reset();

		///* Set scene dirty */
		SetSceneDirty(pScene);
	}

	void EditorSceneManager::Save(UUID uuid, const std::string& path, bool newScene)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetEngine()->GetSceneManager()->GetOpenScene(uuid);
		YAML::Emitter out;
		EditorSceneSerializer::SerializeScene(EditorApplication::GetInstance()->GetEngine(), pScene, out);
		std::ofstream outStream(path);
		outStream << out.c_str();
		outStream.close();
		if (newScene) EditorAssetDatabase::ImportNewScene(path, pScene);
		SetSceneDirty(pScene, false);

		std::stringstream stream;
		stream << "Saved scene to: " << path;
		EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo(stream.str());

		ProjectSpace::Save();
	}
}
