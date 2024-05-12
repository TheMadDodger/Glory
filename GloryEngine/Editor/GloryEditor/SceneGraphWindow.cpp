#include "SceneGraphWindow.h"
#include "Selection.h"
#include "EditorSceneManager.h"
#include "ObjectMenu.h"
#include "ImGuiHelpers.h"
#include "Undo.h"
#include "SetParentAction.h"
#include "SetSiblingIndexAction.h"
#include "EditorUI.h"
#include "EditorAssetDatabase.h"
#include "EntitySceneObjectEditor.h"
#include "EditableEntity.h"
#include "EntityEditor.h"
#include "EditorApplication.h"

#include <Engine.h>
#include <Debug.h>
#include <GScene.h>
#include <SceneManager.h>
#include <IconsFontAwesome6.h>
#include <StringUtils.h>
#include <PrefabData.h>
#include <AssetManager.h>
#include <UUIDRemapper.h>
#include <glm/ext/quaternion_float.hpp>

namespace Glory::Editor
{
	DND DragAndDrop{ { ST_Path, ResourceTypes::GetHash<EditableEntity>(), ResourceTypes::GetHash<PrefabData>() } };

	SceneGraphWindow::SceneGraphWindow() : EditorWindowTemplate("Scene Graph", 300.0f, 680.0f)
	{
	}

	SceneGraphWindow::~SceneGraphWindow()
	{
	}

	void SceneGraphWindow::OnGUI()
	{
		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();

		const GScene* pActiveScene = pScenes->GetActiveScene();

		const ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		const ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		const ImVec2 size = vMax - vMin;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		m_NeedsFilter = EditorUI::SearchBar(ImGui::GetContentRegionAvail().x, SearchBuffer, SearchBufferSize);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Search by object name,\nyou can also search by component by typing c:componenttype");
		}

		if (m_NeedsFilter)
		{
			m_SearchResultExcludeCache.clear();
			if (!std::string_view{SearchBuffer}.empty())
			{
				for (size_t i = 0; i < pScenes->OpenScenesCount(); i++)
				{
					GScene* pScene = pScenes->GetOpenScene(i);
					if (GetExcludedObjectsFromFilterRecursive(pScene)) {
						continue;
					}
					m_SearchResultExcludeCache.push_back(pScene->GetUUID());
				}
			}
			else m_NeedsFilter = false;
		}

		for (size_t i = 0; i < pScenes->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenes->GetOpenScene(i);
			if (IsExcluded(pScene->GetUUID())) continue;
			SceneDropdown(i, pScene, pScene == pActiveScene);
		}

		ImVec2 availableRegion = ImGui::GetContentRegionAvail();
		if (availableRegion.y <= 0.0f)
		{
			ImGui::PopStyleVar();
			return;
		}

		ImGui::InvisibleButton("WINDOWTARGET", { size.x, availableRegion.y });
		DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetActiveScene();
			if (HandleAssetDragAndDrop(0, pScene, dndHash, pPayload)) return;
			if (dndHash != ResourceTypes::GetHash<EditableEntity>()) return;

			const ObjectPayload payload = *(const ObjectPayload*)pPayload->Data;
			pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(payload.SceneID);
			Entity draggingEntity = pScene->GetEntityByEntityID(payload.EntityID);
			Entity oldParent = draggingEntity.ParentEntity();

			Undo::StartRecord("Re-parent", draggingEntity.EntityUUID());
			const UUID oldParentID = oldParent.IsValid() ? oldParent.EntityUUID() : UUID(0);
			const UUID newParent = 0;
			const size_t oldSiblingIndex = draggingEntity.SiblingIndex();
			draggingEntity.SetParent(0);
			Undo::AddAction(new SetParentAction(pScene, oldParentID, newParent, oldSiblingIndex));
			Undo::StopRecord();

			EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
		});

		if (ImGui::IsItemClicked(1))
		{
			ObjectMenu::Open(nullptr, ObjectMenuType::T_Hierarchy);
		}

		ImGui::PopStyleVar();

		m_NeedsFilter = false;
	}

	void SceneGraphWindow::SceneDropdown(size_t index, GScene* pScene, bool isActive)
	{
		ImGui::PushID(int(index));

		const std::hash<std::string> hasher{};
		const size_t hash = hasher((pScene->Name() + std::to_string(pScene->GetUUID())));

		bool selected = Selection::IsObjectSelected(pScene);
		const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Selected;

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.07f, 0.0720f, 0.074f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.07f, 0.0720f, 0.074f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.1525f, 0.1505f, 1.0f));
		if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));

		const std::string label = pScene->Name() + (EditorApplication::GetInstance()->GetSceneManager().IsSceneDirty(pScene) ? " *" : "");
		if (m_NeedsFilter) ImGui::SetNextItemOpen(true);
		const bool nodeOpen = ImGui::TreeNodeEx("##scenenode", node_flags, label.data());
		if (isActive) ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
			if (HandleAssetDragAndDrop(0, pScene, dndHash, pPayload)) return;
			if (dndHash != ResourceTypes::GetHash<EditableEntity>()) return;

			const ObjectPayload payload = *(const ObjectPayload*)pPayload->Data;
			if (pScene->GetUUID() != payload.SceneID)
			{
				EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning("Moving an entity to another scene is currently not supported");
				return;
			}

			Entity draggingEntity = pScene->GetEntityByEntityID(payload.EntityID);
			Entity oldParent = draggingEntity.ParentEntity();

			Undo::StartRecord("Re-parent", draggingEntity.EntityUUID());
			const UUID oldParentID = oldParent.IsValid() ? oldParent.EntityUUID() : UUID(0);
			const UUID newParent = 0;
			const size_t oldSiblingIndex = draggingEntity.SiblingIndex();
			draggingEntity.SetParent(0);
			Undo::AddAction(new SetParentAction(pScene, oldParentID, newParent, oldSiblingIndex));
			Undo::StopRecord();

			EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
		});

		if (ImGui::IsItemClicked())
		{
			Selection::SetActiveObject(pScene);
		}
		else if (ImGui::IsItemClicked(1))
		{
			ObjectMenu::Open(pScene, T_Scene);
		}

		if (nodeOpen)
		{
			//ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);

			size_t index = 0;
			for (size_t i = 0; i < pScene->ChildCount(0); i++)
			{
				Entity childEntity = pScene->ChildEntity(0, i);
				if (!ChildrenList(index, childEntity)) continue;
				++index;
			}

			//ImGui::PopStyleVar();
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	bool SceneGraphWindow::ChildrenList(size_t index, Entity& entity)
	{
		ImGui::PushID(int(index));

		const UUID id = entity.EntityUUID();

		if (IsExcluded(id))
		{
			ImGui::PopID();
			return false;
		}

		// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
		bool selected = Selection::IsObjectSelected(GetEditableEntity(entity.GetEntityID(), entity.GetScene()));
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | (selected ? ImGuiTreeNodeFlags_Selected : 0);

		const std::string name{entity.Name()};

		const size_t childCount = entity.ChildCount();

		if (index == 0)
		{
			ImGui::InvisibleButton("##reorderbefore", ImVec2(ImGui::GetWindowContentRegionWidth(), 2.0f));
			DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
				GScene* pScene = entity.GetScene();
				Entity parentEntity = entity.ParentEntity();
				if (HandleAssetDragAndDrop(parentEntity.GetEntityID(), pScene, dndHash, pPayload)) return;
				if (dndHash != ResourceTypes::GetHash<EditableEntity>()) return;

				const ObjectPayload payload = *(const ObjectPayload*)pPayload->Data;
				if (payload.SceneID != pScene->GetUUID())
				{
					EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning("Moving an entity to another scene is currently not supported");
					return;
				}

				Entity draggingEntity = pScene->GetEntityByEntityID(payload.EntityID);
				if (draggingEntity.GetEntityID() == entity.GetEntityID()) return;

				bool canParent = true;
				while (parentEntity.IsValid())
				{
					if (parentEntity.GetEntityID() == payload.EntityID)
					{
						canParent = false;
						break;
					}

					parentEntity = parentEntity.ParentEntity();
				}

				parentEntity = entity.ParentEntity();
				if (canParent)
				{
					Entity oldParent = draggingEntity.ParentEntity();

					Undo::StartRecord("Re-parent", draggingEntity.EntityUUID());
					const UUID oldParentID = oldParent.IsValid() ? oldParent.EntityUUID() : UUID(0);
					const UUID newParent = parentEntity.IsValid() ? parentEntity.EntityUUID() : UUID(0);
					const size_t oldSiblingIndex = draggingEntity.SiblingIndex();
					const size_t siblingIndex = 0;
					draggingEntity.SetParent(parentEntity.GetEntityID());
					draggingEntity.SetSiblingIndex(siblingIndex);
					Undo::AddAction(new SetParentAction(pScene, oldParentID, newParent, oldSiblingIndex));
					Undo::AddAction(new SetSiblingIndexAction(pScene, oldSiblingIndex, siblingIndex));
					Undo::StopRecord();

					EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
				}
			});
		}

		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 20.0f);
		if (childCount <= 0) node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (m_NeedsFilter) ImGui::SetNextItemOpen(true);
		GScene* pScene = entity.GetScene();
		const bool isPrefab = entity.GetScene()->Prefab(entity.EntityUUID()) || pScene->PrefabChild(entity.EntityUUID());
		const bool node_open = ImGui::TreeNodeEx("##entitynode", node_flags, "");
		ObjectPayload payload{ entity.GetEntityID(), pScene->GetUUID() };
		DND::DragAndDropSource<EditableEntity>(&payload, sizeof(ObjectPayload), [entity]() {
			ImGui::Text("%s: %s", entity.Name().data(), std::to_string(entity.EntityUUID()).data());
		});
		DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
			if (HandleAssetDragAndDrop(entity.GetEntityID(), pScene, dndHash, pPayload)) return;
			if (dndHash != ResourceTypes::GetHash<EditableEntity>()) return;

			const ObjectPayload payload = *(const ObjectPayload*)pPayload->Data;
			if (payload.SceneID != pScene->GetUUID())
			{
				EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning("Moving an entity to another scene is currently not supported");
				return;
			}
			Entity draggingEntity = pScene->GetEntityByEntityID(payload.EntityID);

			if (draggingEntity.GetEntityID() == entity.GetEntityID()) return;

			Entity parentEntity = entity.ParentEntity();
			bool canParent = true;
			while (parentEntity.IsValid())
			{
				if (parentEntity.GetEntityID() == payload.EntityID)
				{
					canParent = false;
					break;
				}

				parentEntity = parentEntity.ParentEntity();
			}
			if (canParent)
			{
				Entity oldParent = draggingEntity.ParentEntity();

				Undo::StartRecord("Re-parent", draggingEntity.EntityUUID());
				const UUID oldParentID = oldParent.IsValid() ? oldParent.EntityUUID() : UUID(0);
				const UUID newParent = entity.IsValid() ? entity.EntityUUID() : UUID(0);
				const size_t oldSiblingIndex = draggingEntity.SiblingIndex();
				draggingEntity.SetParent(entity.GetEntityID());
				Undo::AddAction(new SetParentAction(pScene, oldParentID, newParent, oldSiblingIndex));
				Undo::StopRecord();

				EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
			}
		});

		EditableEntity* pEntity = GetEditableEntity(entity.GetEntityID(), pScene);
		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
		{
			Selection::SetActiveObject(pEntity);
		}
		if (ImGui::IsItemClicked(1))
		{
			Selection::SetActiveObject(pEntity);
			ObjectMenu::Open(pEntity, T_SceneObject);
		}

		ImGui::SameLine();
		EntitySceneObjectEditor::DrawObjectNodeName(entity, isPrefab);

		ImGui::PopStyleVar();

		if (node_open)
		{
			size_t index = 0;
			for (size_t i = 0; i < entity.ChildCount(); i++)
			{
				Entity child = entity.ChildEntity(i);
				if (!ChildrenList(index, child)) continue;
				++index;
			}

			if (childCount > 0) ImGui::TreePop();
		}

		ImGui::InvisibleButton("##reorderafter", ImVec2(ImGui::GetWindowContentRegionWidth(), 2.0f));
		DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
			Entity parent = entity.ParentEntity();
			if (HandleAssetDragAndDrop(parent.GetEntityID(), pScene, dndHash, pPayload)) return;
			if (dndHash != ResourceTypes::GetHash<EditableEntity>()) return;

			const ObjectPayload payload = *(const ObjectPayload*)pPayload->Data;
			if (payload.SceneID != pScene->GetUUID())
			{
				EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning("Moving an entity to another scene is currently not supported");
				return;
			}
			Entity draggingEntity = pScene->GetEntityByEntityID(payload.EntityID);

			if (draggingEntity.GetEntityID() == entity.GetEntityID()) return;

			bool canParent = true;
			while (parent.IsValid())
			{
				if (parent.GetEntityID() == payload.EntityID)
				{
					canParent = false;
					break;
				}

				parent = parent.ParentEntity();
			}

			parent = entity.ParentEntity();
			if (canParent)
			{
				Entity oldParent = draggingEntity.ParentEntity();

				Undo::StartRecord("Re-parent", draggingEntity.EntityUUID());
				const UUID oldParentID = oldParent.IsValid() ? oldParent.EntityUUID() : UUID(0);
				const UUID newParent = parent.IsValid() ? parent.EntityUUID() : UUID(0);
				const size_t oldSiblingIndex = draggingEntity.SiblingIndex();
				const size_t siblingIndex = entity.SiblingIndex() + 1;
				draggingEntity.SetParent(parent.GetEntityID());
				draggingEntity.SetSiblingIndex(siblingIndex);
				Undo::AddAction(new SetParentAction(pScene, oldParentID, newParent, oldSiblingIndex));
				Undo::AddAction(new SetSiblingIndexAction(pScene, oldSiblingIndex, siblingIndex));
				Undo::StopRecord();

				EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
			}
		});

		ImGui::PopID();
		return true;
	}

	bool SceneGraphWindow::GetExcludedObjectsFromFilterRecursive(GScene* pScene)
	{
		bool hasNonFilteredChild = false;
		for (size_t i = 0; i < pScene->ChildCount(0); i++)
		{
			Entity child = pScene->ChildEntity(0, i);
			hasNonFilteredChild |= GetExcludedObjectsFromFilterRecursive(child);
		}
		return hasNonFilteredChild;
	}

	bool SceneGraphWindow::GetExcludedObjectsFromFilterRecursive(Entity& entity)
	{
		const std::string_view search{SearchBuffer};
		const bool searchPassed = true;//EntitySceneObjectEditor::SearchCompare(search, pObject);

		bool hasNonFilteredChild = false;
		for (size_t i = 0; i < entity.ChildCount(); i++)
		{
			Entity childEntity = entity.ChildEntity(i);
			hasNonFilteredChild |= GetExcludedObjectsFromFilterRecursive(childEntity);
		}

		const bool shouldBeExcluded = !searchPassed && !hasNonFilteredChild;
		if (shouldBeExcluded)
			m_SearchResultExcludeCache.push_back(entity.EntityUUID());

		return !shouldBeExcluded;
	}

	bool SceneGraphWindow::IsExcluded(const UUID uuid)
	{
		return std::find(m_SearchResultExcludeCache.begin(), m_SearchResultExcludeCache.end(), uuid)
			!= m_SearchResultExcludeCache.end();
	}

	bool SceneGraphWindow::HandleAssetDragAndDrop(Utils::ECS::EntityID entity, GScene* pScene, uint32_t dndHash, const ImGuiPayload* pPayload)
	{
		if (dndHash != ST_Path && dndHash != ResourceTypes::GetHash<PrefabData>()) return false;

		PrefabData* pPrefab = nullptr;

		const uint32_t prefabHash = ResourceTypes::GetHash<PrefabData>();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		AssetManager& assetManager = pEngine->GetAssetManager();
		if (dndHash == ST_Path)
		{
			const std::string path = (const char*)pPayload->Data;
			const UUID uuid = EditorAssetDatabase::FindAssetUUID(path);
			if (!uuid) return false;
			ResourceMeta meta;
			if (!EditorAssetDatabase::GetAssetMetadata(uuid, meta)) return false;
			ResourceTypes& types = pEngine->GetResourceTypes();
			ResourceType* pResourceType = types.GetResourceType(meta.Hash());

			if (meta.Hash() != ResourceTypes::GetHash<PrefabData>())
			{
				bool found = false;
				for (size_t i = 0; i < types.SubTypeCount(pResourceType); ++i)
				{
					ResourceType* pSubResourceType = types.GetSubType(pResourceType, i);
					if (!pSubResourceType) continue;
					if (pSubResourceType->Hash() != ResourceTypes::GetHash<PrefabData>()) continue;

					pPrefab = pEngine->GetAssetManager().GetAssetImmediate<PrefabData>(uuid);
					found = true;
					break;
				}
				if(!found) return false;
			}

			pPrefab = assetManager.GetAssetImmediate<PrefabData>(uuid);
		}
		else if (dndHash = ResourceTypes::GetHash<PrefabData>())
		{
			const UUID prefabID = *(const UUID*)pPayload->Data;
			pPrefab = assetManager.GetAssetImmediate<PrefabData>(prefabID);
		}

		if (!pPrefab) return false;

		if (!pScene)
			pScene = EditorApplication::GetInstance()->GetSceneManager().GetActiveScene(true);

		Entity entityHandle = pScene->GetEntityByEntityID(entity);
		pScene->InstantiatePrefab(entityHandle.EntityUUID(), pPrefab, glm::vec3{}, glm::quat{0, 0, 0, 1}, glm::vec3{1, 1, 1});
		EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
		return true;
	}
}
