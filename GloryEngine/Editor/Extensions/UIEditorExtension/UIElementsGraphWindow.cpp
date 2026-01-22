#include "UIElementsGraphWindow.h"
#include "UIMainWindow.h"
#include "UIDocument.h"
#include "UIEditorExtension.h"
#include "SetUIParentAction.h"
#include "AddUIElementAction.h"
#include "UIDocumentImporter.h"
#include "DeleteUIElementAction.h"

#include <UIComponents.h>

#include <Undo.h>
#include <DND.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <ImGuiHelpers.h>

#include <IconsFontAwesome6.h>
#include <Shortcuts.h>

namespace Glory::Editor
{
	DND DragAndDrop{ { ResourceTypes::GetHash<UIElementType>() } };

	UIElementsGraphWindow::UIElementsGraphWindow() : EditorWindowTemplate("UI Elements", 600.0f, 600.0f),
		m_OpenRightClickPopup(false), m_RightClickedElement(0)
	{
	}

	UIElementsGraphWindow::~UIElementsGraphWindow()
	{
	}

	void UIElementsGraphWindow::OnGUI()
	{
		UIMainWindow* pMainWindow = GetMainWindow();
		UIDocument* pDocument = pMainWindow->CurrentDocument();
		const UUID documentID = pMainWindow->CurrentDocumentID();
		if (documentID == 0 || !pDocument) return;

		const ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		const ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		const ImVec2 size = vMax - vMin;

		const bool selected = false;
		const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Selected;

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.07f, 0.0720f, 0.074f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.07f, 0.0720f, 0.074f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.1525f, 0.1505f, 1.0f));

		const std::string_view label = pDocument->Name();
		const bool nodeOpen = ImGui::TreeNodeEx("##documentnode", node_flags, label.data());
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(pDocument->OriginalDocumentID());
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::YAMLFileRef& file = pDocumentData->File();
		Utils::NodeValueRef node = **pDocumentData;

		Utils::ECS::EntityRegistry& registry = pDocument->Registry();
		DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
			if (dndHash != ResourceTypes::GetHash<UIElementType>()) return;
			const UIElementType& payload = *(const UIElementType*)pPayload->Data;
			const size_t newSiblingIndex = 0;
			const UUID newParentID = 0;

			if (payload.m_NewEntity)
			{
				/* Create new entity */
				const uint32_t type = (uint32_t)payload.m_EntityID;
				const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(type);
				AddUIElementAction::AddElement(pEngine, pDocument, file, pType->TypeName(), type, newParentID, newSiblingIndex);
				return;
			}

			const Utils::ECS::EntityID draggingEntity = payload.m_EntityID;
			Utils::ECS::EntityID oldParent = registry.GetParent(draggingEntity);
			const UUID toReParent = pDocument->EntityUUID(draggingEntity);
			const UUID oldParentID = oldParent ? pDocument->EntityUUID(oldParent) : 0;
			const size_t oldSiblingIndex = registry.SiblingIndex(draggingEntity);
			Undo::StartRecord("UI Re-parent", pDocument->OriginalDocumentID());
			Undo::AddAction<SetUIParentAction>(toReParent, oldParentID, newParentID, oldSiblingIndex, newSiblingIndex);
			Undo::StopRecord();

			registry.SetParent(draggingEntity, newParentID);
			registry.SetSiblingIndex(draggingEntity, newSiblingIndex);
			SetUIParentAction::StoreDocumentState(pEngine, pDocument, node["Entities"]);
			pDocument->SetDrawDirty();
		});

		UUID& selectedEntity = pMainWindow->SelectedEntity();
		if (nodeOpen)
		{
			Utils::ECS::EntityRegistry& registry = pDocument->Registry();
			for (size_t i = 0; i < registry.ChildCount(0); ++i)
			{
				Utils::ECS::EntityID child = registry.Child(0, i);
				if (!ChildrenList(i == 0, selectedEntity, child, pDocument)) continue;
			}
			ImGui::TreePop();
		}

		ImVec2 availableRegion = ImGui::GetContentRegionAvail();
		if (availableRegion.y <= 0.0f)
		{
			ImGui::PopStyleVar();
			return;
		}

		ImGui::InvisibleButton("WINDOWTARGET", { size.x, availableRegion.y });
		DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
			if (dndHash != ResourceTypes::GetHash<UIElementType>()) return;
			const UIElementType& payload = *(const UIElementType*)pPayload->Data;
			const size_t newSiblingIndex = registry.ChildCount(0);
			const UUID newParentID = 0;

			if (payload.m_NewEntity)
			{
				/* Create new entity */
				const uint32_t type = (uint32_t)payload.m_EntityID;
				const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(type);
				AddUIElementAction::AddElement(pEngine, pDocument, file, pType->TypeName(), type, newParentID, newSiblingIndex);
				return;
			}

			const Utils::ECS::EntityID draggingEntity = payload.m_EntityID;
			Utils::ECS::EntityID oldParent = registry.GetParent(draggingEntity);
			const UUID toReParent = pDocument->EntityUUID(draggingEntity);
			const UUID oldParentID = oldParent ? pDocument->EntityUUID(oldParent) : 0;
			const size_t oldSiblingIndex = registry.SiblingIndex(draggingEntity);
			Undo::StartRecord("UI Re-parent", pDocument->OriginalDocumentID());
			Undo::AddAction<SetUIParentAction>(toReParent, oldParentID, newParentID, oldSiblingIndex, newSiblingIndex);
			Undo::StopRecord();
			
			registry.SetParent(draggingEntity, newParentID);
			registry.SetSiblingIndex(draggingEntity, newSiblingIndex);
			SetUIParentAction::StoreDocumentState(pEngine, pDocument, node["Entities"]);
			pDocument->SetDrawDirty();
		});

		ImGui::PopStyleVar();

		if (m_OpenRightClickPopup)
		{
			ImGui::OpenPopup("ElementRightClick");
			m_OpenRightClickPopup = false;
		}

		if (ImGui::BeginPopup("ElementRightClick"))
		{
			if (ImGui::MenuItem("Delete", Shortcuts::GetShortcutString("Delete").data(), false))
			{
				if (selectedEntity == m_RightClickedElement) selectedEntity = 0;
				DeleteUIElementAction::DeleteElement(pEngine, pDocument, file, m_RightClickedElement);
				m_RightClickedElement = 0;
			}
			if (ImGui::MenuItem("Duplicate", Shortcuts::GetShortcutString("Duplicate").data(), false))
			{
				if (selectedEntity == m_RightClickedElement) selectedEntity = 0;
				selectedEntity = AddUIElementAction::DuplicateElement(pEngine, pDocument, file, m_RightClickedElement);
				m_RightClickedElement = 0;
			}
			ImGui::EndPopup();
		}
	}

	UIMainWindow* UIElementsGraphWindow::GetMainWindow()
	{
		return static_cast<UIMainWindow*>(m_pOwner);
	}

	bool UIElementsGraphWindow::ChildrenList(bool first, UUID& selectedEntity, Utils::ECS::EntityID entity, UIDocument* pDocument)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->Registry();

		ImGui::PushID(int(entity));
		const UUID uuid = pDocument->EntityUUID(entity);

		// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
		const bool selected = selectedEntity == uuid;
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | (selected ? ImGuiTreeNodeFlags_Selected : 0);

		const std::string_view name{ pDocument->Name(entity) };

		const size_t childCount = registry.ChildCount(entity);

		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(pDocument->OriginalDocumentID());
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::YAMLFileRef& file = pDocumentData->File();
		Utils::NodeValueRef node = **pDocumentData;

		if (first)
		{
			ImGui::InvisibleButton("##reorderbefore", ImVec2(ImGui::GetWindowContentRegionWidth(), 2.0f));
			DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
				if (dndHash != ResourceTypes::GetHash<UIElementType>()) return;
				const UIElementType& payload = *(const UIElementType*)pPayload->Data;
				Utils::ECS::EntityID parent = registry.GetParent(entity);
				const UUID newParentID = parent ? pDocument->EntityUUID(parent) : 0;
				const size_t newSiblingIndex = 0;

				if (payload.m_NewEntity)
				{
					/* Create new entity */
					const uint32_t type = (uint32_t)payload.m_EntityID;
					const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(type);
					AddUIElementAction::AddElement(pEngine, pDocument, file, pType->TypeName(), type, newParentID, newSiblingIndex);
					return;
				}

				const Utils::ECS::EntityID draggingEntity = payload.m_EntityID;
				if (draggingEntity == entity) return;

				bool canParent = true;
				while (parent != 0)
				{
					if (parent == draggingEntity)
					{
						canParent = false;
						break;
					}
					parent = registry.GetParent(parent);
				}

				if (canParent)
				{
					parent = registry.GetParent(entity);
					Utils::ECS::EntityID oldParent = registry.GetParent(draggingEntity);
					const UUID toReParent = pDocument->EntityUUID(draggingEntity);
					const UUID oldParentID = oldParent ? pDocument->EntityUUID(oldParent) : 0;
					const size_t oldSiblingIndex = registry.SiblingIndex(draggingEntity);
					Undo::StartRecord("UI Re-parent", pDocument->OriginalDocumentID());
					Undo::AddAction<SetUIParentAction>(toReParent, oldParentID, newParentID, oldSiblingIndex, newSiblingIndex);
					Undo::StopRecord();

					registry.SetParent(draggingEntity, parent);
					registry.SetSiblingIndex(draggingEntity, newSiblingIndex);
					SetUIParentAction::StoreDocumentState(pEngine, pDocument, node["Entities"]);
					pDocument->SetDrawDirty();
				}
			});
		}

		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 20.0f);
		if (childCount <= 0) node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		const bool node_open = ImGui::TreeNodeEx("##entitynode", node_flags, "");
		UIElementType payload{ false, entity };
		DND::DragAndDropSource<UIElementType>(&payload, sizeof(UIElementType), [entity, name]() {
			ImGui::Text("%s: %s", name.data(), std::to_string(entity).data());
		});
		DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
			if (dndHash != ResourceTypes::GetHash<UIElementType>()) return;
			const UIElementType& payload = *(const UIElementType*)pPayload->Data;
			Utils::ECS::EntityID parent = registry.GetParent(entity);
			const UUID newParentID = pDocument->EntityUUID(entity);
			const size_t newSiblingIndex = registry.ChildCount(entity);

			if (payload.m_NewEntity)
			{
				/* Create new entity */
				const uint32_t type = (uint32_t)payload.m_EntityID;
				const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(type);
				AddUIElementAction::AddElement(pEngine, pDocument, file, pType->TypeName(), type, newParentID, newSiblingIndex);
				return;
			}

			const Utils::ECS::EntityID draggingEntity = payload.m_EntityID;
			if (draggingEntity == entity) return;
			bool canParent = true;
			while (parent != 0)
			{
				if (parent == draggingEntity)
				{
					canParent = false;
					break;
				}
				parent = registry.GetParent(parent);
			}
			if (canParent)
			{
				parent = registry.GetParent(entity);
				Utils::ECS::EntityID oldParent = registry.GetParent(draggingEntity);
				const UUID toReParent = pDocument->EntityUUID(draggingEntity);
				const UUID oldParentID = oldParent ? pDocument->EntityUUID(oldParent) : 0;
				const size_t oldSiblingIndex = registry.SiblingIndex(draggingEntity);
				Undo::StartRecord("UI Re-parent", pDocument->OriginalDocumentID());
				Undo::AddAction<SetUIParentAction>(toReParent, oldParentID, newParentID, oldSiblingIndex, newSiblingIndex);
				Undo::StopRecord();

				registry.SetParent(draggingEntity, entity);
				SetUIParentAction::StoreDocumentState(pEngine, pDocument, node["Entities"]);
				pDocument->SetDrawDirty();
			}
		});

		//EditableEntity* pEntity = GetEditableEntity(entity.GetEntityID(), pScene);
		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
		{
			selectedEntity = uuid;
		}
		if (ImGui::IsItemClicked(1))
		{
			selectedEntity = uuid;
			m_OpenRightClickPopup = true;
			m_RightClickedElement = uuid;
		}

		Utils::ECS::EntityView* pEntity = registry.GetEntityView(entity);

		ImGui::SameLine();
		ImGui::Text(" %s %s", pEntity->HierarchyActive() ? ICON_FA_EYE : ICON_FA_EYE_SLASH, name.data());

		ImGui::PopStyleVar();

		if (node_open)
		{
			for (size_t i = 0; i < pEntity->ChildCount(); i++)
			{
				Utils::ECS::EntityID child = pEntity->Child(i);
				if (!ChildrenList(i == 0, selectedEntity, child, pDocument)) continue;
			}

			if (childCount > 0) ImGui::TreePop();
		}

		ImGui::InvisibleButton("##reorderafter", ImVec2(ImGui::GetWindowContentRegionWidth(), 2.0f));
		DragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
			if (dndHash != ResourceTypes::GetHash<UIElementType>()) return;
			const UIElementType& payload = *(const UIElementType*)pPayload->Data;
			Utils::ECS::EntityID parent = registry.GetParent(entity);
			const UUID newParentID = parent ? pDocument->EntityUUID(parent) : 0;
			const size_t newSiblingIndex = registry.SiblingIndex(entity) + 1;

			if (payload.m_NewEntity)
			{
				/* Create new entity */
				const uint32_t type = (uint32_t)payload.m_EntityID;
				const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(type);
				AddUIElementAction::AddElement(pEngine, pDocument, file, pType->TypeName(), type, newParentID, newSiblingIndex);
				return;
			}

			const Utils::ECS::EntityID draggingEntity = payload.m_EntityID;
			if (draggingEntity == entity) return;

			bool canParent = true;
			while (parent != 0)
			{
				if (parent == draggingEntity)
				{
					canParent = false;
					break;
				}
				parent = registry.GetParent(parent);
			}
			parent = registry.GetParent(entity);

			if (canParent)
			{
				parent = registry.GetParent(entity);
				Utils::ECS::EntityID oldParent = registry.GetParent(draggingEntity);
				const UUID toReParent = pDocument->EntityUUID(draggingEntity);
				const UUID oldParentID = oldParent ? pDocument->EntityUUID(oldParent) : 0;
				const size_t oldSiblingIndex = registry.SiblingIndex(draggingEntity);
				Undo::StartRecord("UI Re-parent", pDocument->OriginalDocumentID());
				Undo::AddAction<SetUIParentAction>(toReParent, oldParentID, newParentID, oldSiblingIndex, newSiblingIndex);
				Undo::StopRecord();

				registry.SetParent(draggingEntity, parent);
				registry.SetSiblingIndex(draggingEntity, newSiblingIndex);
				SetUIParentAction::StoreDocumentState(pEngine, pDocument, node["Entities"]);
				pDocument->SetDrawDirty();
			}
		});

		ImGui::PopID();
		return true;
	}
}
