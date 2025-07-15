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
		const bool oldActive = selectedEntity["Active"].As<bool>();
		bool active = oldActive;
		bool change = false;
		if (EditorUI::CheckBox("Active", &active))
		{
			Undo::StartRecord("UI Property");
			Undo::ApplyYAMLEdit(file, selectedEntity["Active"].Path(), oldActive, active);
			Undo::StopRecord();
			change = true;
		}

		change |= NameGUI();
		ImGui::Spacing();

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
				Undo::StartRecord("UI Property Change", selected, true);

				auto structData = component["Properties"];

				const TypeData* pStructTypeData = pType;
				for (size_t i = 0; i < pStructTypeData->FieldCount(); ++i)
				{
					const FieldData* pFieldData = pStructTypeData->GetFieldData(i);
					auto field = structData[pFieldData->Name()];
					const uint32_t fieldFlags = uint32_t(Reflect::GetFieldFlags(pFieldData));
					change |= PropertyDrawer::DrawProperty(file, field.Path(), pFieldData->Type(), pFieldData->ArrayElementType(), fieldFlags);
				}
				Undo::StopRecord();
			}

			/*if (ImGui::BeginPopup("ComponentRightClick"))
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
			}*/
			ImGui::PopID();
			ImGui::Spacing();
		}
		ImGui::Separator();
		/*const float buttonWidth = ImGui::GetContentRegionAvail().x;
		if (ImGui::Button("Add Component", { buttonWidth, 0.0f }))
		{
			
		}

		if (removeComponent)
		{
			components.Remove(toRemoveComponent);
			change = true;
		}*/

		ImGui::PopID();
		ImGui::PopID();

		if (change)
		{
			EditorAssetDatabase::SetAssetDirty(documentID);
			pDocument->SetDrawDirty();
		}
	}

	UIMainWindow* UIElementInspector::GetMainWindow()
	{
		return static_cast<UIMainWindow*>(m_pOwner);
	}

	bool UIElementInspector::NameGUI()
	{
		UIMainWindow* pMainWindow = GetMainWindow();
		UIDocument* pDocument = pMainWindow->CurrentDocument();
		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		EditorResourceManager& resources = pApp->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(pDocument->OriginalDocumentID());
		YAMLResource<UIDocumentData>* pDocumentData = static_cast<YAMLResource<UIDocumentData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocumentData;
		const UUID& selected = pMainWindow->SelectedEntity();
		const std::string uuidString = std::to_string(selected);
		auto entities = file["Entities"];
		auto selectedEntity = entities[uuidString];
		auto name = selectedEntity["Name"];

		ImGui::PushID("Object");
		ImGui::TextDisabled("UI Element");
		ImGui::Separator();
		ImGui::Text("UUID:");
		const float textWitdh = ImGui::CalcTextSize(uuidString.data()).x;
		ImGui::SameLine();
		const ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ cursorPos.x + ImGui::GetContentRegionAvail().x - textWitdh, cursorPos.y });
		ImGui::Text(uuidString.data());
		Undo::StartRecord("Change Element Name", pDocument->OriginalDocumentID());
		const bool change = EditorUI::InputText(file, name.Path());
		Undo::StopRecord();
		ImGui::PopID();
		return change;
	}
}
