#include "EntitySceneObjectEditor.h"
#include "AddComponentAction.h"
#include "RemoveComponentAction.h"
#include <imgui.h>
#include <string>
#include <SceneObjectNameAction.h>
#include <Undo.h>

namespace Glory::Editor
{
	EntitySceneObjectEditor::EntitySceneObjectEditor() : m_NameBuff(""), m_Initialized(false), m_AddingComponent(false), m_pObject(nullptr)
	{
	}

	EntitySceneObjectEditor::~EntitySceneObjectEditor()
	{
		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [](Editor* pEditor) { Editor::ReleaseEditor(pEditor); });
		m_pComponentEditors.clear();

		std::for_each(m_pComponents.begin(), m_pComponents.end(), [](EntityComponentObject* pObject) { delete pObject; });
		m_pComponents.clear();
	}

	bool EntitySceneObjectEditor::OnGUI()
	{
		if (!m_Initialized) Initialize();
		m_pObject = (SceneObject*)m_pTarget;

		const std::string uuidString = std::to_string(m_pObject->GetUUID());
		ImGui::PushID(uuidString.c_str());
		bool change = NameGUI();
		change |= ComponentGUI();
		ImGui::PopID();
		return change;
	}

	void EntitySceneObjectEditor::Refresh()
	{
		Initialize();
	}

	void EntitySceneObjectEditor::Initialize()
	{
		std::for_each(m_pComponents.begin(), m_pComponents.end(), [](EntityComponentObject* pObject) { delete pObject; });
		m_pComponents.clear();

		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [](Editor* pEditor) { Editor::ReleaseEditor(pEditor); });
		m_pComponentEditors.clear();

		EntitySceneObject* pObject = (EntitySceneObject*)m_pTarget;
		Entity entity = pObject->GetEntityHandle();
		EntityID entityID = entity.GetEntityID();
		EntityView* pEntityView = entity.GetEntityView();

		for (size_t i = 0; i < pEntityView->ComponentCount(); i++)
		{
			UUID uuid = pEntityView->ComponentUUIDAt(i);
			size_t componentType = pEntityView->ComponentTypeAt(i);
			EntityComponentObject* pComponentObject = new EntityComponentObject(entityID, uuid, componentType, entity.GetScene()->GetRegistry());
			m_pComponents.push_back(pComponentObject);
			Editor* pEditor = Editor::CreateEditor(pComponentObject);
			if (pEditor) m_pComponentEditors.push_back(pEditor);
		}

		m_Initialized = true;
	}

	bool EntitySceneObjectEditor::NameGUI()
	{
		std::string originalName = m_pObject->Name();
		const char* name = originalName.c_str();
		memcpy(m_NameBuff, name, originalName.length() + 1);
		m_NameBuff[originalName.length()] = '\0';

		UUID uuid = m_pObject->GetUUID();
		std::string uuidString = std::to_string(uuid);
		ImGui::Text(uuidString.data());
		ImGui::Text("Name");
		ImGui::SameLine();
		bool change = ImGui::InputText("##Name", m_NameBuff, MAXNAMESIZE);
		m_pObject->SetName(m_NameBuff);
		if (change)
		{
			Undo::StartRecord("Change Name", m_pObject->GetUUID());
			Undo::AddAction(new SceneObjectNameAction(originalName, m_pObject->Name()));
			Undo::StopRecord();
		}

		return change;
	}

	bool EntitySceneObjectEditor::ComponentGUI()
	{
		ImGui::PushID("Components");
		bool change = false;

		Entity entity = ((EntitySceneObject*)m_pObject)->GetEntityHandle();
		EntityID entityID = entity.GetEntityID();
		GloryECS::EntityRegistry* pRegistry = entity.GetScene()->GetRegistry();

		int index = 0;
		const std::string& nameString = m_pObject->Name();
		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [&](Editor* pEditor)
		{
			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

			std::string id = nameString + std::to_string(index);
			std::hash<std::string> hasher;
			size_t hash = hasher(id);

			ImGui::PushID(id.c_str());
			if (ImGui::TreeNodeEx((void*)hash, node_flags, pEditor->Name().data()))
			{
				change |= pEditor->OnGUI();
				ImGui::TreePop();
			}

			if (ImGui::IsItemClicked(1))
			{
				m_RightClickedComponentIndex = index;
				ImGui::OpenPopup("ComponentRightClick");
			}
			ImGui::PopID();

			++index;
		});
		ImGui::PopID();

		if (ImGui::Button("Add Component"))
		{
			EntityComponentPopup::Open(pRegistry);
			m_AddingComponent = true;
		}
		
		if (m_AddingComponent)
		{
			size_t toAddTypeHash = EntityComponentPopup::GetLastSelectedComponentTypeHash();
			if (toAddTypeHash)
			{
				Undo::StartRecord("Add Component", m_pTarget->GetUUID());
				pRegistry->CreateComponent(entityID, toAddTypeHash, UUID());
				//pRegistry->GetSystems()->CreateComponent(entityID, toAddTypeHash);
				//size_t index = m_pComponentEditors.size();
				//EntityComponentData* pComponentData = pRegistry->GetEntityComponentDataAt(entityID, index);
				//Undo::AddAction(new AddComponentAction(toAddTypeHash, pComponentData->GetComponentUUID(), index));
				Undo::StopRecord();
				m_AddingComponent = false;
				Initialize();
				change = true;
			}
		}
		
		m_ComponentPopup.OnGUI();
		
		//if (ImGui::BeginPopup("ComponentRightClick"))
		//{
		//	if (ImGui::MenuItem("Remove"))
		//	{
		//		EntityComponentData* pComponentData = pRegistry->GetEntityComponentDataAt(entityID, m_RightClickedComponentIndex);
		//		Undo::StartRecord("Remove Component", m_pTarget->GetUUID());
		//		EntitySceneObject* pObject = (EntitySceneObject*)m_pTarget;
		//		EntityScene* pScene = (EntityScene*)pObject->GetScene();
		//		Undo::AddAction(new RemoveComponentAction(pScene, pComponentData, m_RightClickedComponentIndex));
		//		pRegistry->RemoveComponent(entityID, m_RightClickedComponentIndex);
		//		Undo::StopRecord();
		//		Initialize();
		//		change = true;
		//	}
		//
		//	ImGui::EndPopup();
		//}
		return change;
	}
}
