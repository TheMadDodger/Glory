#include "UIElementInspector.h"
#include "UIMainWindow.h"

#include <UIDocument.h>

#include <EditableResource.h>
#include <EditorAssetDatabase.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <PropertyDrawer.h>

namespace Glory::Editor
{
	UIElementInspector::UIElementInspector() : EditorWindowTemplate("UI Inspector", 600.0f, 600.0f)
	{
	}

	UIElementInspector::~UIElementInspector()
	{
	}

	/*bool NameGUI(Utils::ECS::EntityID entityID)
	{
		ImGui::PushID("Object");

		ImGui::TextDisabled("UI Element");
		ImGui::Separator();

		const std::string_view originalName = entity.Name();
		const char* name = originalName.data();
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
		const bool change = EditorUI::InputText("Name", m_NameBuff, MAXNAMESIZE, ImGuiInputTextFlags_EnterReturnsTrue);
		if (change)
		{
			pScene->SetEntityName(entityID, m_NameBuff);
		}

		ImGui::PopID();
		return change;
	}*/

	void UIElementInspector::OnGUI()
	{
		UIMainWindow* pMainWindow = GetMainWindow();
		UIDocument* pDocument = pMainWindow->CurrentDocument();
		const UUID documentID = pMainWindow->CurrentDocumentID();
		if (!documentID || !pDocument)
		{
			ImGui::TextUnformatted("No UI document open");
			return;
		}

		const UUID& selected = pMainWindow->SelectedEntity();
		if (!selected || !pDocument->EntityExists(selected))
		{
			ImGui::TextUnformatted("No UI element selected");
			return;
		}

		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(pDocument->OriginalDocumentID());
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocumentData;

		const std::string uuidString = std::to_string(selected);

		auto entities = file["Entities"];
		auto selectedEntity = entities[uuidString];

		ImGui::PushID(uuidString.c_str());
		bool active = selectedEntity["Active"].As<bool>();
		bool change = false;
		if (EditorUI::CheckBox("Active", &active))
		{
			selectedEntity["Active"].Set(active);
			change = true;
		}
		/*change |= NameGUI();
		ImGui::Spacing();
		change |= ComponentGUI();*/

		ImGui::PushID("Components");
		ImGui::TextDisabled("Components");
		ImGui::Separator();

		bool removeComponent = false;
		size_t toRemoveComponent = 0;
		auto components = selectedEntity["Components"];
		for (size_t i = 0; i < components.Size(); ++i)
		{
			ImGui::PushID(i);
			auto component = components[i];
			const uint32_t type = component["TypeHash"].As<uint32_t>();
			const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(type);

			const std::string_view label = pType->TypeName();
			const bool open = EditorUI::Header(label);
			
			if (ImGui::IsItemClicked(1))
			{
				ImGui::OpenPopup("ComponentRightClick");
			}

			if (open)
			{
				Undo::StartRecord("Property Change", selected, true);
				change |= PropertyDrawer::DrawProperty(file, component["Properties"].Path(), pType->InternalTypeHash(), pType->TypeHash(), 0);
				Undo::StopRecord();
			}

			if (ImGui::BeginPopup("ComponentRightClick"))
			{
				const bool removeAllowed = i != 0;
				if (ImGui::MenuItem("Remove", "", false, removeAllowed))
				{
					removeComponent = true;
					toRemoveComponent = i;
				}
				if (!removeAllowed && ImGui::IsItemHovered())
					ImGui::SetTooltip("You cannot remove the Transform of an entity");

				ImGui::EndPopup();
			}
			ImGui::PopID();
			ImGui::Spacing();
		}
		ImGui::Separator();
		const float buttonWidth = ImGui::GetContentRegionAvail().x;
		if (ImGui::Button("Add Component", { buttonWidth, 0.0f }))
		{
			
		}

		if (removeComponent)
		{
			components.Remove(toRemoveComponent);
			change = true;
		}

		ImGui::PopID();
		ImGui::PopID();

		if (change)
			EditorAssetDatabase::SetAssetDirty(documentID);
	}

	UIMainWindow* UIElementInspector::GetMainWindow()
	{
		return static_cast<UIMainWindow*>(m_pOwner);
	}
}
