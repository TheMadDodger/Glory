#include "SceneGraphWindow.h"
//#include <SceneManager.h>
//#include <GameScene.h>
#include "Selection.h"
#include "EditorSceneManager.h"

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
		//GameScene* pScene = SceneManager::GetInstance()->GetActiveScene();
		
		//m_I = 0;
		//
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
		//for (size_t i = 0; i < EditorSceneManager::OpenSceneCount(); i++)
		//{
		//	GameScene* pActive = EditorSceneManager::GetActiveScene();
		//	GameScene* pScene = EditorSceneManager::GetOpenSceneAt(i);
		//	bool isActive = pActive == pScene;
		//	SceneDropdown(pScene, isActive);
		//}
	}

	//void SceneGraphWindow::SceneDropdown(GameScene* pScene, bool isActive)
	//{
	//	if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
	//	if (ImGui::TreeNode(pScene->GetName().c_str()))
	//	{
	//		if (isActive) ImGui::PopStyleColor();
	//		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);
	//
	//		for (size_t i = 0; i < pScene->GetChildCount(); i++)
	//		{
	//			ChildrenList(pScene->GetChild(i));
	//		}
	//
	//		ImGui::PopStyleVar();
	//		ImGui::TreePop();
	//	}
	//	else
	//	{
	//		if (isActive) ImGui::PopStyleColor();
	//	}
	//}
	//
	//void SceneGraphWindow::ChildrenList(Spartan::GameObject* pChild)
	//{
	//	// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
	//	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (pChild->IsSelected() ? ImGuiTreeNodeFlags_Selected : 0);
	//
	//	auto childrenOfChild = pChild->GetChildren();
	//	if (!childrenOfChild.empty())
	//	{
	//		bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)m_I, node_flags, pChild->GetName());
	//		if (ImGui::IsItemClicked())
	//		{
	//			//if (m_CurrentSelectedObject != pChild)
	//				//ComponentParameterManager::GetInstance()->Clear();
	//
	//			Selection::SetActiveObject(pChild);
	//		}
	//		if (ImGui::IsItemClicked(1))
	//		{
	//			m_CurrentRightClickedObject = pChild;
	//			ImGui::OpenPopup("object_menu_popup" + m_I);
	//		}
	//		if (ImGui::BeginPopup("object_menu_popup" + m_I))
	//		{
	//			ObjectMenu();
	//			ImGui::EndPopup();
	//		}
	//		++m_I;
	//		// Node
	//		if (node_open)
	//		{
	//			for (auto pChildOfChild : childrenOfChild)
	//			{
	//				ChildrenList(pChildOfChild);
	//			}
	//
	//			ImGui::TreePop();
	//		}
	//	}
	//	else
	//	{
	//		// Leaf: The only reason we have a TreeNode at all is to allow selection of the leaf. Otherwise we can use BulletText() or TreeAdvanceToLabelPos()+Text().
	//		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
	//		ImGui::TreeNodeEx((void*)(intptr_t)m_I, node_flags, pChild->GetName());
	//		if (ImGui::IsItemClicked())
	//		{
	//			Selection::SetActiveObject(pChild);
	//		}
	//		if (ImGui::IsItemClicked(1))
	//		{
	//			m_CurrentRightClickedObject = pChild;
	//			ImGui::OpenPopup("object_menu_popup" + m_I);
	//		}
	//		if (ImGui::BeginPopup("object_menu_popup" + m_I))
	//		{
	//			ObjectMenu();
	//			ImGui::EndPopup();
	//		}
	//		++m_I;
	//	}
	//}
	//
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
