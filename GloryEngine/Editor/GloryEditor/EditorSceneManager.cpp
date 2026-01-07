#include "EditorSceneManager.h"
#include "Selection.h"
#include "EditorAssetDatabase.h"
#include "EditorSceneSerializer.h"
#include "CreateObjectAction.h"
#include "Undo.h"
#include "EditorApplication.h"
#include "EntityEditor.h"
#include "AssetCompiler.h"

#include <Debug.h>
#include <Engine.h>
#include <AssetDatabase.h>
#include <SceneManager.h>
#include <TitleBar.h>
#include <tinyfiledialogs.h>
#include <ProjectSpace.h>
#include <GScene.h>
#include <Dispatcher.h>

namespace Glory::Editor
{
	EditorSceneManager::EditorSceneManager(EditorApplication* pApplication):
		m_pApplication(pApplication), SceneManager(pApplication->GetEngine())
	{}

	EditorSceneManager::~EditorSceneManager() = default;

#pragma region Scene manager overrides

	GScene* EditorSceneManager::NewScene(const std::string& name, bool)
	{
		/* New scene is always additive in the editor */
		GScene* pNewScene = new GScene();
		m_pOpenScenes.emplace_back(pNewScene);
		m_OpenedSceneIDs.emplace_back(pNewScene->GetUUID());
		m_SceneFiles.emplace_back(YAMLResource<GScene>{}).SetPath("NewScene.gscene");
		pNewScene->SetManager(this);
		SetSceneDirty(pNewScene);
		SetupCallbacks(pNewScene);
		return pNewScene;
	}

	void EditorSceneManager::OnLoadScene(UUID uuid)
	{
		AssetLocation location;
		EditorAssetDatabase::GetAssetLocation(uuid, location);
		const std::string path = std::string{ EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath() } + "\\" + location.Path;

		const size_t index = m_SceneFiles.size();
		m_SceneFiles.emplace_back(YAMLResource<GScene>{path});
		YAMLResource<GScene>& yamlFile = m_SceneFiles[index];

		std::filesystem::path filePath = path;
		GScene* pScene = EditorSceneSerializer::DeserializeScene(EditorApplication::GetInstance()->GetEngine(), (*yamlFile).RootNodeRef().ValueRef(), uuid, filePath.filename().replace_extension().string());
		if (pScene == nullptr) return;
		pScene->SetResourceUUID(uuid);
		m_pOpenScenes.emplace_back(pScene);
		m_OpenedSceneIDs.emplace_back(uuid);

		if (m_pApplication->IsInPlayMode())
		{
			pScene->GetRegistry().EnableAllIndividualCallbacks();
			pScene->Start();
		}

		GScene* pActiveScene = SceneManager::GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");

		SceneEventsDispatcher().Dispatch({ SceneEventType::Opened, uuid });
	}

	void EditorSceneManager::OnUnloadScene(GScene* pScene)
	{
		// TODO: Check if scene has changes
		const UUID uuid = pScene->GetUUID();
		Selection::SetActiveObject(nullptr);
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		if (it == m_OpenedSceneIDs.end()) return;
		const size_t index = it - m_OpenedSceneIDs.begin();
		m_OpenedSceneIDs.erase(it);
		m_SceneFiles.erase(m_SceneFiles.begin() + index);
		GScene* pActiveScene = SceneManager::GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
		SceneEventsDispatcher().Dispatch({ SceneEventType::Closed, uuid });
	}

	void EditorSceneManager::OnUnloadAllScenes()
	{
		m_OpenedSceneIDs.clear();
		m_SceneFiles.clear();
	}

#pragma endregion

	GScene* EditorSceneManager::OpenSceneInMemory(UUID uuid)
	{
		AssetLocation location;
		EditorAssetDatabase::GetAssetLocation(uuid, location);
		std::string path = std::string{ EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath() } + "\\" + location.Path;
		Utils::YAMLFileRef file{ path };
		std::filesystem::path filePath = path;
		GScene* pScene = EditorSceneSerializer::DeserializeScene(EditorApplication::GetInstance()->GetEngine(), file.RootNodeRef().ValueRef(), uuid,
			filePath.filename().replace_extension().string(), EditorSceneSerializer::NoComponentCallbacks);
		return pScene;
	}

	void EditorSceneManager::OpenScene(GScene* pScene, UUID uuid)
	{
		if (m_pApplication->IsInPlayMode()) return;

		if (GetOpenScene(uuid))
			CloseScene(uuid);

		SetupCallbacks(pScene);

		pScene->SetResourceUUID(uuid);
		m_pOpenScenes.emplace_back(pScene);
		m_OpenedSceneIDs.emplace_back(uuid);

		if (m_pApplication->IsInPlayMode())
		{
			pScene->GetRegistry().EnableAllIndividualCallbacks();
			pScene->Start();
		}
		else
		{
			pScene->GetRegistry().InvokeAll(Utils::ECS::InvocationType::OnEnableDraw,
			[](Utils::ECS::BaseTypeView* pTypeView, Utils::ECS::EntityView* pEntity, size_t componentIndex) {
				const bool isActive = pEntity->IsActive() && pTypeView->IsActiveByIndex(componentIndex);
				return isActive;
			});
		}

		AssetLocation location;
		if (EditorAssetDatabase::GetAssetLocation(uuid, location))
		{
			const std::string path = std::string{ EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath() } + "\\" + location.Path;
			m_SceneFiles.emplace_back(YAMLResource<GScene>{path});
		}
		else
		{
			m_SceneFiles.emplace_back(YAMLResource<GScene>{}).SetPath("NewScene.gscene");;
		}

		GScene* pActiveScene = SceneManager::GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
		SceneEventsDispatcher().Dispatch({ SceneEventType::Opened, uuid });
	}

	void EditorSceneManager::OpenScene(UUID uuid, bool additive)
	{
		if (m_pApplication->IsInPlayMode()) return;

		if (GetOpenScene(uuid))
			CloseScene(uuid);

		if (!additive)
			CloseAllScenes();

		AssetLocation location;
		EditorAssetDatabase::GetAssetLocation(uuid, location);
		const std::string path = std::string{ EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath() } + "\\" + location.Path;

		const size_t index = m_SceneFiles.size();
		m_SceneFiles.emplace_back(YAMLResource<GScene>{path});
		YAMLResource<GScene>& yamlFile = m_SceneFiles[index];

		std::filesystem::path filePath = path;
		GScene* pScene = EditorSceneSerializer::DeserializeScene(EditorApplication::GetInstance()->GetEngine(), (*yamlFile).RootNodeRef().ValueRef(), uuid, filePath.filename().replace_extension().string());

		if (pScene == nullptr) return;
		pScene->SetResourceUUID(uuid);
		m_pOpenScenes.emplace_back(pScene);
		m_OpenedSceneIDs.emplace_back(uuid);

		pScene->GetRegistry().InvokeAll(Utils::ECS::InvocationType::OnEnableDraw,
		[](Utils::ECS::BaseTypeView* pTypeView, Utils::ECS::EntityView* pEntity, size_t componentIndex) {
			const bool isActive = pEntity->IsActive() && pTypeView->IsActiveByIndex(componentIndex);
			return isActive;
		});

		GScene* pActiveScene = SceneManager::GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
		SceneEventsDispatcher().Dispatch({ SceneEventType::Opened, uuid });
	}

	void EditorSceneManager::SaveOpenScenes()
	{
		std::for_each(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), [this](UUID uuid)
		{
			AssetLocation location;
			if (!EditorAssetDatabase::GetAssetLocation(uuid, location)) return; // new scene

			if (location.Path == "") return;
			Save(uuid, std::string{ EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath() } + "\\" + location.Path);
		});
	}

	void EditorSceneManager::CloseAllScenes()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		m_ActiveSceneIndex = 0;
		m_OpenedSceneIDs.clear();
		m_DirtySceneIDs.clear();
		m_SceneFiles.clear();
		TitleBar::SetText("Scene", "No Scene open");
	}

	void EditorSceneManager::CloseScene(UUID uuid)
	{
		// TODO: Check if scene has changes
		Selection::SetActiveObject(nullptr);
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		if (it == m_OpenedSceneIDs.end()) return;
		const size_t index = it - m_OpenedSceneIDs.begin();
		GScene* pScene = GetOpenScene(uuid);
		pScene->Stop();
		SetSceneDirty(pScene, false);
		m_OpenedSceneIDs.erase(it);
		m_pOpenScenes.erase(m_pOpenScenes.begin() + index);
		m_SceneFiles.erase(m_SceneFiles.begin() + index);
		delete pScene;

		GScene* pActiveScene = SceneManager::GetActiveScene();
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");

		SceneEventsDispatcher().Dispatch({ SceneEventType::Closed, uuid });
	}

	UUID EditorSceneManager::GetOpenSceneUUID(size_t index)
	{
		return m_OpenedSceneIDs[index];
	}

	void EditorSceneManager::SaveScene(UUID uuid)
	{
		if (!GetOpenScene(uuid)) return;
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
		const char* path = tinyfd_saveFileDialog("Save Scene",
			EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath().data(),
			1, filters, "Glory Scene");

		if (!path) return;

		std::filesystem::path relativePath = path;
		relativePath = relativePath.lexically_relative(
			EditorApplication::GetInstance()->GetEngine()->GetAssetDatabase().GetAssetPath()
		);
		const UUID existingAsset = EditorAssetDatabase::FindAssetUUID(relativePath.string());

		if (existingAsset != 0)
		{
			Save(m_CurrentlySavingScene, path);
			CloseScene(m_CurrentlySavingScene);
			OpenScene(existingAsset, true);
			return;
		}

		Save(m_CurrentlySavingScene, path, true);
	}

	void EditorSceneManager::SerializeOpenScenes(Utils::InMemoryYAML out)
	{
		auto scenes = out.RootNodeRef().ValueRef();
		scenes.SetSequence();
		for (size_t i = 0; i < m_pOpenScenes.size(); i++)
		{
			GScene* pScene = m_pOpenScenes[i];
			const UUID uuid = pScene->GetUUID();

			scenes.PushBack(YAML::Node(YAML::NodeType::Map));

			auto scene = scenes[i];
			scene["Name"].Set(pScene->Name());
			scene["UUID"].Set(uint64_t(pScene->GetUUID()));

			EditorSceneSerializer::SerializeScene(EditorApplication::GetInstance()->GetEngine(), pScene, scene["Scene"]);
		}
	}

	void EditorSceneManager::OpenAllFromYAML(Utils::InMemoryYAML data)
	{
		auto root = data.RootNodeRef().ValueRef();
		for (size_t i = 0; i < root.Size(); i++)
		{
			auto sceneDataNode = root[i];
			auto nameNode = sceneDataNode["Name"];
			std::string name = nameNode.As<std::string>();
			auto uuidNode = sceneDataNode["UUID"];
			UUID uuid = uuidNode.As<uint64_t>();
			auto sceneNode = sceneDataNode["Scene"];
			GScene* pScene = EditorSceneSerializer::DeserializeScene(EditorApplication::GetInstance()->GetEngine(), sceneNode, uuid, name);
			OpenScene(pScene, uuid);
		}
	}

	void EditorSceneManager::SetSceneDirty(GScene* pScene, bool dirty)
	{
		if (!pScene) return;
		if (dirty && !GetOpenScene(pScene->GetUUID())) return;
		auto itor = std::find(m_DirtySceneIDs.begin(), m_DirtySceneIDs.end(), pScene->GetUUID());
		if (dirty)
		{
			if (itor != m_DirtySceneIDs.end()) return; // Already dirty
			m_DirtySceneIDs.emplace_back(pScene->GetUUID());
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
		if (!pScene) pScene = GetActiveScene();
		if (!pScene) pScene = NewScene();

		/* Serialize the objects entire heirarchy */
		Utils::InMemoryYAML data;
		auto root = data.RootNodeRef().ValueRef();
		root.SetSequence();
		EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, entity.GetEntityID(), root);

		/* Deserialize node into a new objects */
		PasteSceneObject(pScene, entity.Parent(), root);
	}

	void EditorSceneManager::PasteSceneObject(GScene* pScene, Utils::ECS::EntityID parent, Utils::NodeValueRef entities)
	{
		/* Deserialize node into a new objects */
		Utils::ECS::EntityID newEntityID = 0;
		Entity parentEntity = pScene->GetEntityByEntityID(parent);
		for (size_t i = 0; i < entities.Size(); i++)
		{
			Utils::NodeValueRef entity = entities[i];
			if (i == 0 && parentEntity.IsValid())
			{
				const UUID parentUUID = parentEntity.EntityUUID();
				entity["ParentUUID"].Set((uint64_t)parentUUID);
				EditorApplication::GetInstance()->GetEngine()->m_UUIDRemapper.EnforceRemap(parentUUID, parentUUID);
			}

			Entity newEntity = EditorSceneSerializer::DeserializeEntity(EditorApplication::GetInstance()->GetEngine(), pScene, entity, EditorSceneSerializer::Flags::GenerateNewUUIDs);
			if (i == 0 && newEntity.IsValid())
			{
				Undo::StartRecord("Duplicate", newEntity.EntityUUID());
				Undo::AddAction<CreateObjectAction>(pScene);
				Undo::StopRecord();
				newEntityID = newEntity.GetEntityID();
			}
		}

		EditorApplication::GetInstance()->GetEngine()->m_UUIDRemapper.Reset();

		/* Set scene dirty */
		SetSceneDirty(pScene);

		if (newEntityID)
		{
			EditableEntity* pEntity = GetEditableEntity(newEntityID, pScene);
			Selection::SetActiveObject((Object*)pEntity);
		}
	}

	EditorSceneManager::SceneEventDispatcher& EditorSceneManager::SceneEventsDispatcher()
	{
		static EditorSceneManager::SceneEventDispatcher dispatcher;
		return dispatcher;
	}

	void EditorSceneManager::OnInitialize()
	{
		Undo::RegisterChangeHandler(".gscene", "Settings", [this](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			for (size_t i = 0; i < m_SceneFiles.size(); ++i)
			{
				if (m_SceneFiles[i].Path() != file.Path()) continue;
				GScene* pScene = GetOpenScene(i);
				SetSceneDirty(pScene);
				if (!AssetCompiler::CompileSceneSettings(pScene->GetUUID()))
					m_pEngine->GetDebug().LogError("Failed to re-compile scene settings");
				break;
			}
		});
	}

	void EditorSceneManager::OnCleanup()
	{
		m_OpenedSceneIDs.clear();
		m_DirtySceneIDs.clear();
		m_SceneFiles.clear();
		m_CurrentlySavingScene = 0;
	}

	YAMLResource<GScene>* EditorSceneManager::GetSceneFile(UUID uuid)
	{
		for (size_t i = 0; i < m_OpenedSceneIDs.size(); ++i)
		{
			if (m_OpenedSceneIDs[i] != uuid) continue;
			return &m_SceneFiles[i];
		}
		return nullptr;
	}

	void EditorSceneManager::OnSetActiveScene(GScene* pActiveScene)
	{
		TitleBar::SetText("Scene", pActiveScene ? pActiveScene->Name().c_str() : "No Scene open");
	}

	void EditorSceneManager::Save(UUID uuid, const std::string& path, bool newScene)
	{
		auto yamlResource = GetSceneFile(uuid);
		yamlResource->SetPath(path);
		auto& file = **yamlResource;
		auto root = file.RootNodeRef().ValueRef();

		GScene* pScene = GetOpenScene(uuid);
		EditorSceneSerializer::SerializeScene(EditorApplication::GetInstance()->GetEngine(), pScene, root);
		yamlResource->Save();

		if (newScene) EditorAssetDatabase::ImportNewScene(path, pScene);
		SetSceneDirty(pScene, false);

		for (size_t i = 0; i < m_OpenedSceneIDs.size(); ++i)
		{
			if (m_OpenedSceneIDs[i] != uuid) continue;
			m_SceneFiles[i].Reload(path);
			break;
		}

		std::stringstream stream;
		stream << "Saved scene to: " << path;
		EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo(stream.str());

		ProjectSpace::Save();
	}

	void EditorSceneManager::SetupCallbacks(GScene* pScene)
	{
		if (EditorApplication::GetInstance()->CurrentMode() == EditorMode::M_Play) return;
		Utils::ECS::EntityRegistry& pRegistry = pScene->GetRegistry();
		pRegistry.SetCallbackEnabled(Utils::ECS::InvocationType::OnEnable, false);
		pRegistry.SetCallbackEnabled(Utils::ECS::InvocationType::OnDisable, false);
		pRegistry.SetCallbackEnabled(Utils::ECS::InvocationType::Start, false);
		pRegistry.SetCallbackEnabled(Utils::ECS::InvocationType::Stop, false);
	}
}
