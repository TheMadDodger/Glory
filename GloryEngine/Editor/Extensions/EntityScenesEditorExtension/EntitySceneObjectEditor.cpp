#include "EntitySceneObjectEditor.h"
#include "AddComponentAction.h"
#include "RemoveComponentAction.h"
#include <imgui.h>
#include <string>
#include <SceneObjectNameAction.h>
#include <Undo.h>
#include <algorithm>
#include <EditorUI.h>
#include <string_view>
#include <ResourceType.h>

#include <Components.h>

#include "FontAwesome/IconsFontAwesome6.h"

namespace Glory::Editor
{
	const std::map<size_t, std::string_view> COMPONENT_ICONS = {
		{ ResourceType::GetHash<Transform>(), ICON_FA_LOCATION_CROSSHAIRS },
		{ ResourceType::GetHash<MeshFilter>(), ICON_FA_CUBE },
		{ ResourceType::GetHash<MeshRenderer>(), ICON_FA_CUBES },
		{ ResourceType::GetHash<CameraComponent>(), ICON_FA_VIDEO },
		{ ResourceType::GetHash<LayerComponent>(), ICON_FA_LAYER_GROUP },
		{ ResourceType::GetHash<ScriptedComponent>(), ICON_FA_FILE_CODE },
	};

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
		ImGui::Spacing();
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
		ImGui::PushID("Object");

		ImGui::TextDisabled("Entity Object");
		ImGui::Separator();

		std::string originalName = m_pObject->Name();
		const char* name = originalName.c_str();
		memcpy(m_NameBuff, name, originalName.length() + 1);
		m_NameBuff[originalName.length()] = '\0';

		const UUID uuid = m_pObject->GetUUID();
		std::string uuidString = std::to_string(uuid);
		ImGui::Text("UUID:");
		const float textWitdh = ImGui::CalcTextSize(uuidString.data()).x;
		ImGui::SameLine();
		const ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ cursorPos.x + ImGui::GetContentRegionAvail().x - textWitdh, cursorPos.y });
		ImGui::Text(uuidString.data());
		const bool change = EditorUI::InputText("Name", m_NameBuff, MAXNAMESIZE);
		if (change)
		{
			m_pObject->SetName(m_NameBuff);
			Undo::StartRecord("Change Name", m_pObject->GetUUID());
			Undo::AddAction(new SceneObjectNameAction(originalName, m_pObject->Name()));
			Undo::StopRecord();
		}

		ImGui::PopID();
		return change;
	}

	bool EntitySceneObjectEditor::ComponentGUI()
	{
		ImGui::PushID("Components");

		ImGui::TextDisabled("Components");
		ImGui::Separator();

		bool change = false;

		Entity entity = ((EntitySceneObject*)m_pObject)->GetEntityHandle();
		EntityID entityID = entity.GetEntityID();
		GloryECS::EntityRegistry* pRegistry = entity.GetScene()->GetRegistry();

		int index = 0;
		const std::string& nameString = m_pObject->Name();
		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [&](Editor* pEditor)
		{
			std::string id = nameString + std::to_string(index);
			std::hash<std::string> hasher;
			size_t hash = hasher(id);

			std::string_view icon = "";
			const size_t componentHash = m_pComponents[index]->ComponentType();
			if(COMPONENT_ICONS.find(componentHash) != COMPONENT_ICONS.end())
				icon = COMPONENT_ICONS.at(componentHash);

			ImGui::PushID(index);

			std::string label = std::string(icon) + "	" + pEditor->Name();
			if (EditorUI::Header(label))
				change |= pEditor->OnGUI();

			if (ImGui::IsItemClicked(1))
			{
				m_RightClickedComponentIndex = index;
				ImGui::OpenPopup("ComponentRightClick");
			}
			ImGui::PopID();

			ImGui::Spacing();

			++index;
		});
		ImGui::PopID();

		ImGui::Separator();
		const float buttonWidth = ImGui::GetContentRegionAvail().x;
		if (ImGui::Button("Add Component", { buttonWidth, 0.0f }))
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
