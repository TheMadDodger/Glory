#include "SceneGraphWindow.h"
#include "Selection.h"
#include "EditorSceneManager.h"
#include "ObjectMenu.h"
#include "ImGuiHelpers.h"

#include <Game.h>
#include <Engine.h>

#include "Undo.h"
#include "SetParentAction.h"
#include "SetSiblingIndexAction.h"

namespace Glory::Editor
{
	SceneGraphWindow::SceneGraphWindow() : EditorWindowTemplate("Scene Graph", 300.0f, 680.0f)
	{
	}

	SceneGraphWindow::~SceneGraphWindow()
	{
	}

	void SceneGraphWindow::OnGUI()
	{
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();

		const GScene* pActiveScene = pScenesModule->GetActiveScene();

		const ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		const ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		const ImVec2 size = vMax - vMin;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			SceneDropdown(i, pScene, pScene == pActiveScene);
		}

		ImVec2 availableRegion = ImGui::GetContentRegionAvail();
		if (availableRegion.y <= 0.0f)
		{
			ImGui::PopStyleVar();
			return;
		}

		ImGui::InvisibleButton("WINDOWTARGET", { size.x, availableRegion.y });
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
			{
				const DNDPayload payload = *(const DNDPayload*)pPayload->Data;

				Undo::StartRecord("Re-parent", payload.pObject->GetUUID());
				const UUID oldParent = payload.pObject->GetParent() ? payload.pObject->GetParent()->GetUUID() : UUID(0);
				const UUID newParent = 0;
				const size_t oldSiblingIndex = payload.pObject->GetSiblingIndex();
				payload.pObject->SetParent(nullptr);
				Undo::AddAction(new SetParentAction(oldParent, newParent, oldSiblingIndex));
				Undo::StopRecord();

				EditorSceneManager::SetSceneDirty(payload.pObject->GetScene());
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked(1))
		{
			ObjectMenu::Open(nullptr, ObjectMenuType::T_Hierarchy);
		}

		ImGui::PopStyleVar();
	}

	void SceneGraphWindow::SceneDropdown(size_t index, GScene* pScene, bool isActive)
	{
		const std::hash<std::string> hasher{};
		const uint32_t hash = hasher((pScene->Name() + std::to_string(pScene->GetUUID())));

		bool selected = Selection::IsObjectSelected(pScene);
		const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Selected;

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.07f, 0.0720f, 0.074f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.07f, 0.0720f, 0.074f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.1525f, 0.1505f, 1.0f));
		if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));

		const std::string label = pScene->Name() + (EditorSceneManager::IsSceneDirty(pScene) ? " *" : "");
		const bool nodeOpen = ImGui::TreeNodeEx((void*)hash, node_flags, label.data());
		if (isActive) ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
			{
				const DNDPayload payload = *(const DNDPayload*)pPayload->Data;

				Undo::StartRecord("Re-parent", payload.pObject->GetUUID());
				const UUID oldParent = payload.pObject->GetParent() ? payload.pObject->GetParent()->GetUUID() : UUID(0);
				const UUID newParent = 0;
				const size_t oldSiblingIndex = payload.pObject->GetSiblingIndex();
				payload.pObject->SetParent(nullptr);
				Undo::AddAction(new SetParentAction(oldParent, newParent, oldSiblingIndex));
				Undo::StopRecord();

				EditorSceneManager::SetSceneDirty(payload.pObject->GetScene());
			}
			ImGui::EndDragDropTarget();
		}

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

			for (size_t i = 0; i < pScene->SceneObjectsCount(); i++)
			{
				SceneObject* pObject = pScene->GetSceneObject(i);
				if (pObject->GetParent() != nullptr) continue;
				ChildrenList(i, pObject);
			}

			//ImGui::PopStyleVar();
			ImGui::TreePop();
			return;
		}
	}

	void SceneGraphWindow::ChildrenList(size_t index, SceneObject* pObject)
	{
		// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.

		bool selected = Selection::IsObjectSelected(pObject);
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | (selected ? ImGuiTreeNodeFlags_Selected : 0);

		const UUID id = pObject->GetUUID();

		constexpr std::hash<std::string> hasher{};
		const std::string nameAndIDMinus1 = pObject->Name() + std::to_string(id - 1);
		const std::string nameAndID = pObject->Name() + std::to_string(id);
		const std::string nameAndIDPlus1 = pObject->Name() + std::to_string(id + 1);
		const uint32_t hash = hasher(nameAndID);

		const size_t childCount = pObject->ChildCount();

		if (index == 0)
		{
			ImGui::InvisibleButton(nameAndIDMinus1.c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), 2.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
				{
					const DNDPayload payload = *(const DNDPayload*)pPayload->Data;
					SceneObject* pParent = pObject->GetParent();
					bool canParent = true;
					while (pParent)
					{
						if (pParent == payload.pObject)
						{
							canParent = false;
							break;
						}

						pParent = pParent->GetParent();
					}

					pParent = pObject->GetParent();
					if (canParent)
					{
						Undo::StartRecord("Re-parent", payload.pObject->GetUUID());
						const UUID oldParent = payload.pObject->GetParent() ? payload.pObject->GetParent()->GetUUID() : UUID(0);
						const UUID newParent = pParent ? pParent->GetUUID() : UUID(0);
						const size_t oldSiblingIndex = payload.pObject->GetSiblingIndex();
						const size_t siblingIndex = pObject->GetSiblingIndex();
						payload.pObject->SetParent(pParent);
						payload.pObject->SetSiblingIndex(siblingIndex);
						Undo::AddAction(new SetParentAction(oldParent, newParent, oldSiblingIndex));
						Undo::AddAction(new SetSiblingIndexAction(oldSiblingIndex, siblingIndex));
						Undo::StopRecord();

						EditorSceneManager::SetSceneDirty(payload.pObject->GetScene());
					}
				}
				ImGui::EndDragDropTarget();
			}
		}

		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 20.0f);
		if (childCount <= 0) node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		const bool node_open = ImGui::TreeNodeEx((void*)hash, node_flags, pObject->Name().data());
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			// Set payload to carry the index of our item (could be anything)
			DNDPayload payload{ pObject };
			ImGui::SetDragDropPayload("DND_DEMO_CELL", &payload, sizeof(DNDPayload));

			// Display preview (could be anything, e.g. when dragging an image we could decide to display
			// the filename and a small preview of the image, etc.)
			ImGui::Text(pObject->Name().data());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
			{
				const DNDPayload payload = *(const DNDPayload*)pPayload->Data;

				SceneObject* pParent = pObject->GetParent();
				bool canParent = true;
				while (pParent)
				{
					if (pParent == payload.pObject)
					{
						canParent = false;
						break;
					}

					pParent = pParent->GetParent();
				}
				if (canParent)
				{
					Undo::StartRecord("Re-parent", payload.pObject->GetUUID());
					const UUID oldParent = payload.pObject->GetParent() ? payload.pObject->GetParent()->GetUUID() : UUID(0);
					const UUID newParent = pObject ? pObject->GetUUID() : UUID(0);
					const size_t oldSiblingIndex = payload.pObject->GetSiblingIndex();
					payload.pObject->SetParent(pObject);
					Undo::AddAction(new SetParentAction(oldParent, newParent, oldSiblingIndex));
					Undo::StopRecord();

					EditorSceneManager::SetSceneDirty(payload.pObject->GetScene());
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked())
		{
			Selection::SetActiveObject(pObject);
		}
		if (ImGui::IsItemClicked(1))
		{
			ObjectMenu::Open(pObject, T_SceneObject);
		}

		if (node_open)
		{
			for (size_t i = 0; i < pObject->ChildCount(); i++)
			{
				SceneObject* pChild = pObject->GetChild(i);
				ChildrenList(i, pChild);
			}

			if (childCount > 0) ImGui::TreePop();
		}

		ImGui::InvisibleButton(nameAndIDPlus1.c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), 2.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
			{
				const DNDPayload payload = *(const DNDPayload*)pPayload->Data;
				SceneObject* pParent = pObject->GetParent();
				bool canParent = true;
				while (pParent)
				{
					if (pParent == payload.pObject)
					{
						canParent = false;
						break;
					}

					pParent = pParent->GetParent();
				}

				pParent = pObject->GetParent();
				if (canParent)
				{
					Undo::StartRecord("Re-parent", payload.pObject->GetUUID());
					const UUID oldParent = payload.pObject->GetParent() ? payload.pObject->GetParent()->GetUUID() : UUID(0);
					const UUID newParent = pParent ? pParent->GetUUID() : UUID(0);
					const size_t oldSiblingIndex = payload.pObject->GetSiblingIndex();
					const size_t siblingIndex = pObject->GetSiblingIndex();
					payload.pObject->SetParent(pParent);
					payload.pObject->SetSiblingIndex(siblingIndex);
					Undo::AddAction(new SetParentAction(oldParent, newParent, oldSiblingIndex));
					Undo::AddAction(new SetSiblingIndexAction(oldSiblingIndex, siblingIndex));
					Undo::StopRecord();

					EditorSceneManager::SetSceneDirty(payload.pObject->GetScene());
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
}
