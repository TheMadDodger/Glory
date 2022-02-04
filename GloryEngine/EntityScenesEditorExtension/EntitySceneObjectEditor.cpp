#include "EntitySceneObjectEditor.h"
#include <imgui.h>
#include <string>

namespace Glory::Editor
{
	EntitySceneObjectEditor::EntitySceneObjectEditor() : m_NameBuff(""), m_Initialized(false), m_AddingComponent(false), m_pObject(nullptr)
	{
	}

	EntitySceneObjectEditor::~EntitySceneObjectEditor()
	{
		std::for_each(m_pComponents.begin(), m_pComponents.end(), [](EntityComponentObject* pObject) { delete pObject; });
		m_pComponents.clear();
	}

	void EntitySceneObjectEditor::OnGUI()
	{
		if (!m_Initialized) Initialize();
		m_pObject = (SceneObject*)m_pTarget;

		NameGUI();
		ComponentGUI();
	}

	void EntitySceneObjectEditor::Initialize()
	{
		std::for_each(m_pComponents.begin(), m_pComponents.end(), [](EntityComponentObject* pObject) { delete pObject; });
		m_pComponents.clear();

		m_pComponentEditors.clear();
		EntitySceneObject* pObject = (EntitySceneObject*)m_pTarget;
		pObject->GetEntityHandle().ForEachComponent([&](Registry* pRegistry, EntityID entityID, EntityComponentData* pComponentData)
		{
			EntityComponentObject* pComponentObject = new EntityComponentObject(pComponentData, pRegistry);
			m_pComponents.push_back(pComponentObject);
			Editor* pEditor = Editor::CreateEditor(pComponentObject);
			if (pEditor) m_pComponentEditors.push_back(pEditor);
		});
		m_Initialized = true;
	}

	void EntitySceneObjectEditor::NameGUI()
	{
		const std::string& nameString = m_pObject->Name();
		const char* name = nameString.c_str();
		memcpy(m_NameBuff, name, nameString.length() + 1);
		m_NameBuff[nameString.length()] = '\0';

		UUID uuid = m_pObject->GetUUID();
		std::string uuidString = std::to_string(uuid);
		ImGui::Text(uuidString.data());
		ImGui::Text("Name");
		ImGui::SameLine();
		ImGui::InputText("##Name", m_NameBuff, MAXNAMESIZE);
		m_pObject->SetName(m_NameBuff);
	}

	void EntitySceneObjectEditor::ComponentGUI()
	{
		Entity entity = ((EntitySceneObject*)m_pObject)->GetEntityHandle();
		EntityID entityID = entity.GetEntityID();
		Registry* pRegistry = entity.GetScene()->GetRegistry();

		int index = 0;
		const std::string& nameString = m_pObject->Name();
		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [&](Editor* pEditor)
		{
			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

			std::string id = nameString + std::to_string(index);
			std::hash<std::string> hasher;
			size_t hash = hasher(id);

			bool nodeOpen = ImGui::TreeNodeEx((void*)hash, node_flags, pEditor->Name().data());

			if (ImGui::IsItemClicked(1))
			{
				m_RightClickedComponentIndex = index;
				ImGui::OpenPopup("ComponentRightClick");
			}

			if (nodeOpen)
			{
				pEditor->OnGUI();
				ImGui::TreePop();
			}

			++index;
		});

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
				pRegistry->GetSystems()->CreateComponent(entityID, toAddTypeHash);
				m_AddingComponent = false;
				Initialize();
			}
		}

		m_ComponentPopup.OnGUI();

		if (ImGui::BeginPopup("ComponentRightClick"))
		{
			if (ImGui::MenuItem("Remove"))
			{
				pRegistry->RemoveComponent(entityID, m_RightClickedComponentIndex);
				Initialize();
			}

			ImGui::EndPopup();
		}
	}
}
