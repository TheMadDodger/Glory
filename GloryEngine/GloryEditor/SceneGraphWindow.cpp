#include "SceneGraphWindow.h"
#include "Selection.h"
#include "EditorSceneManager.h"
#include "Game.h"
#include "Selection.h"
#include <ImGuiHelpers.h>

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

		GScene* pActiveScene = pScenesModule->GetActiveScene();

		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		ImVec2 size = vMax - vMin;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			SceneDropdown(i, pScene, pScene == pActiveScene);
		}
		ImGui::PopStyleVar();


		ImGui::InvisibleButton("WINDOWTARGET", size);
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
			{
				DNDPayload payload = *(const DNDPayload*)pPayload->Data;
				payload.pObject->SetParent(nullptr);
			}
			ImGui::EndDragDropTarget();
		}

		//if (ImGui::IsItemClicked(1))
		//{
		//	m_CurrentRightClickedObject = nullptr;
		//	ImGui::OpenPopup("object_menu_popup");
		//}
		//
		//if (ImGui::BeginPopup("object_menu_popup"))
		//{
		//	ObjectMenu();
		//	ImGui::EndPopup();
		//}
		//
	}

	void SceneGraphWindow::SceneDropdown(size_t index, GScene* pScene, bool isActive)
	{
		std::hash<std::string> hasher;
		size_t hash = hasher((pScene->Name() + std::to_string(pScene->GetUUID())));

		bool selected = Selection::IsObjectSelected(pScene);
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Selected;

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.07f, 0.0720f, 0.074f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.07f, 0.0720f, 0.074f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.1525f, 0.1505f, 1.0f));
		if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		bool nodeOpen = ImGui::TreeNodeEx((void*)hash, node_flags, pScene->Name().data());
		if (isActive) ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
			{
				DNDPayload payload = *(const DNDPayload*)pPayload->Data;
				payload.pObject->SetParent(nullptr);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked())
		{
			Selection::SetActiveObject(pScene);
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

		UUID id = pObject->GetUUID();

		std::hash<std::string> hasher;
		std::string nameAndIDMinus1 = pObject->Name() + std::to_string(id - 1);
		std::string nameAndID = pObject->Name() + std::to_string(id);
		std::string nameAndIDPlus1 = pObject->Name() + std::to_string(id + 1);
		size_t hash = hasher(nameAndID);

		size_t childCount = pObject->ChildCount();

		if (index == 0)
		{
			ImGui::InvisibleButton(nameAndIDMinus1.c_str(), ImVec2(ImGui::GetWindowContentRegionWidth(), 2.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
				{
					DNDPayload payload = *(const DNDPayload*)pPayload->Data;
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
						payload.pObject->SetParent(pParent);
						payload.pObject->SetBeforeObject(pObject);
					}
				}
				ImGui::EndDragDropTarget();
			}
		}

		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 20.0f);
		if (childCount <= 0) node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		bool node_open = ImGui::TreeNodeEx((void*)hash, node_flags, pObject->Name().data());
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
				DNDPayload payload = *(const DNDPayload*)pPayload->Data;

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
				if (canParent) payload.pObject->SetParent(pObject);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked())
		{
			//if (m_CurrentSelectedObject != pChild)
				//ComponentParameterManager::GetInstance()->Clear();

			Selection::SetActiveObject(pObject);
		}
		if (ImGui::IsItemClicked(1))
		{
			//m_CurrentRightClickedObject = pChild;
			//ImGui::OpenPopup("object_menu_popup" + index);
		}
		if (ImGui::BeginPopup("object_menu_popup" + id))
		{
			//ObjectMenu();
			//ImGui::EndPopup();
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
				DNDPayload payload = *(const DNDPayload*)pPayload->Data;
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
					payload.pObject->SetParent(pParent);
					payload.pObject->SetAfterObject(pObject);
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Node
		//if (node_open)
		//{
		//	for (auto pChildOfChild : childrenOfChild)
		//	{
		//		ChildrenList(pChildOfChild);
		//	}
		//
		//	ImGui::TreePop();
		//}
		//}
		//else
		//{
		//	node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
		//	ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, pChild->GetName());
		//	if (ImGui::IsItemClicked())
		//	{
		//		Selection::SetActiveObject(pChild);
		//	}
		//	if (ImGui::IsItemClicked(1))
		//	{
		//		m_CurrentRightClickedObject = pChild;
		//		ImGui::OpenPopup("object_menu_popup" + m_I);
		//	}
		//	if (ImGui::BeginPopup("object_menu_popup" + m_I))
		//	{
		//		ObjectMenu();
		//		ImGui::EndPopup();
		//	}
		//	++m_I;
		//}
	}

	//void SceneGraphWindow::ObjectMenu()
	//{
	//	Spartan::GameObject *pCreatedObj = nullptr;
	//	ImGui::MenuItem("Object Menu", NULL, false, false);
	//
	//	if (ImGui::MenuItem("Copy", "Ctrl+C"))
	//	{
	//
	//	}
	//
	//	if (ImGui::MenuItem("Paste", "Ctrl+V"))
	//	{
	//
	//	}
	//
	//	if (ImGui::MenuItem("Duplicate"))
	//	{
	//
	//	}
	//
	//	if (ImGui::MenuItem("Delete"))
	//	{
	//		if (m_CurrentRightClickedObject)
	//		{
	//			auto pParent = m_CurrentRightClickedObject->GetParent();
	//			//if (pParent)
	//				//pParent->RemoveChild(m_CurrentRightClickedObject, true);
	//			//else
	//				//RemoveChild(m_CurrentRightClickedObject, true);
	//
	//			Selection::Clear();
	//		}
	//	}
	//
	//	if (ImGui::BeginMenu("Create"))
	//	{
	//		if (ImGui::MenuItem("Empty"))
	//		{
	//			pCreatedObj = new GameObject("Empty Game Object");
	//		}
	//		if (ImGui::BeginMenu("Geometry"))
	//		{
	//			if (ImGui::MenuItem("Box"))
	//			{
	//
	//			}
	//			if (ImGui::MenuItem("Circle"))
	//			{
	//
	//			}
	//			if (ImGui::MenuItem("Skinned Mesh"))
	//			{
	//
	//			}
	//			ImGui::EndMenu();
	//		}
	//		if (ImGui::BeginMenu("Effects"))
	//		{
	//			if (ImGui::MenuItem("Particle Emitter"))
	//			{
	//
	//			}
	//			if (ImGui::MenuItem("Camera"))
	//			{
	//
	//			}
	//			ImGui::EndMenu();
	//		}
	//		if (ImGui::BeginMenu("UI"))
	//		{
	//			if (ImGui::MenuItem("UI Container"))
	//			{
	//
	//			}
	//			if (ImGui::MenuItem("UI Button"))
	//			{
	//
	//			}
	//			ImGui::EndMenu();
	//		}
	//		if (ImGui::BeginMenu("Animation"))
	//		{
	//			if (ImGui::MenuItem("Rig"))
	//			{
	//				//pCreatedObj = new SkeletonObject(new Skeleton());
	//			}
	//			if (ImGui::MenuItem("Bone"))
	//			{
	//				//pCreatedObj = new Bone(100.0f, 0.872664625f);
	//			}
	//			ImGui::EndMenu();
	//		}
	//		ImGui::EndMenu();
	//	}
	//
	//	//if (pCreatedObj)
	//	//{
	//	//	if (m_CurrentRightClickedObject)
	//	//	{
	//	//		auto skeleton = dynamic_cast<SkeletonObject*>(m_CurrentRightClickedObject);
	//	//		auto parentBone = dynamic_cast<Bone*>(m_CurrentRightClickedObject);
	//	//		auto bone = static_cast<Bone*>(pCreatedObj);
	//	//
	//	//		if (skeleton)
	//	//		{
	//	//			if (bone)
	//	//			{
	//	//				skeleton->GetSkeleton()->AddBone(bone);
	//	//				AddChild(bone);
	//	//			}
	//	//			else
	//	//			{
	//	//				// Only bones are allowed as children on a skeleton!
	//	//				delete pCreatedObj;
	//	//			}
	//	//		}
	//	//		else if (parentBone)
	//	//		{
	//	//			if (bone)
	//	//			{
	//	//				parentBone->AddChildBone(bone);
	//	//				parentBone->AddChild(bone);
	//	//			}
	//	//			else
	//	//			{
	//	//				// Only bones are allowed as children on a skeleton!
	//	//				delete pCreatedObj;
	//	//			}
	//	//		}
	//	//		else
	//	//		{
	//	//			m_CurrentRightClickedObject->AddChild(pCreatedObj);
	//	//		}
	//	//	}
	//	//	else AddChild(pCreatedObj);
	//	//}
	//}
}
