#include "ObjectPicker.h"
#include "EditorUI.h"
#include "DND.h"
#include "EditableEntity.h"
#include "EditorApplication.h"

#include <WindowModule.h>
#include <Engine.h>
#include <imgui.h>
#include <algorithm>
#include <EditorAssetDatabase.h>

#include <IconsFontAwesome6.h>
#include <EditorSceneManager.h>

namespace Glory::Editor
{
	DND DragAndDropTarget = { { ResourceTypes::GetHash<EditableEntity>() }};

	char ObjectPicker::m_FilterBuffer[200] = "";
	std::string ObjectPicker::m_Filter = "";
	std::vector<Entity> ObjectPicker::m_FilteredObjects;

	bool ObjectPicker::ObjectDropdown(const std::string& label, SceneObjectRef* value, const float borderPadding)
	{
		return ObjectDropdown(label, value->SceneUUIDMember(), value->ObjectUUIDMember(), borderPadding);
	}

	bool ObjectPicker::ObjectDropdown(const std::string& label, UUID* sceneValue, UUID* objectValue, const float borderPadding)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(*sceneValue);
		Entity entity = pScene ? pScene->GetEntityByUUID(*objectValue) : Entity{};

		ImGui::PushID(label.c_str());
		std::string objectName = "";
		bool missing = *sceneValue != 0 && *objectValue != 0 && (pScene == nullptr || !entity.IsValid());

		const std::string name{entity.Name()};
		objectName = missing ? "Missing Object" : (!entity.IsValid() ? "Noone" : name + ": " + std::to_string(entity.EntityUUID()));

		bool openPopup = false;
		float start, width;
		bool change = false;
		EditorUI::EmptyDropdown(EditorUI::MakeCleanName(label), objectName, [&]
		{
			m_FilterBuffer[0] = '\0';
			m_Filter = "";
			m_FilteredObjects.clear();
			openPopup = true;
		}, start, width, borderPadding);
		change = DragAndDropTarget.HandleDragAndDropTarget([&](uint32_t, const ImGuiPayload* payload)
		{
			const ObjectPayload objectPayload = *(const ObjectPayload*)payload->Data;
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(objectPayload.SceneID);
			Entity draggingEntity = pScene->GetEntityByEntityID(objectPayload.EntityID);
			*sceneValue = pScene->GetUUID();
			*objectValue = draggingEntity.EntityUUID();
		});

		if (openPopup)
		{
			ImGui::OpenPopup("ObjectPicker");
			RefreshFilter();
		}
		openPopup = false;

		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		Window* pWindow = EditorApplication::GetInstance()->GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
		int mainWindowWidth, mainWindowHeight;
		pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
		ImGui::SetNextWindowPos({ windowPos.x + start, windowPos.y + cursor.y - 2.5f });
		ImGui::SetNextWindowSize({ width, mainWindowHeight - windowPos.y - cursor.y - 10.0f });
		change |= DrawPopup(sceneValue, objectValue);
		ImGui::PopID();
		return change;
	}

	ObjectPicker::ObjectPicker() {}
	ObjectPicker::~ObjectPicker() {}

	void ObjectPicker::RefreshFilter()
	{
		m_FilteredObjects.clear();
		for (size_t i = 0; i < EditorApplication::GetInstance()->GetSceneManager().OpenScenesCount(); ++i)
		{
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(i);
			for (size_t j = 0; j < pScene->ChildCount(0); ++j)
			{
				Entity child = pScene->ChildEntity(0, j);
				if (!m_Filter.empty() && child.Name().find(m_Filter) == std::string::npos &&
					std::to_string(child.EntityUUID()).find(m_Filter) == std::string::npos) continue;
				m_FilteredObjects.push_back(child);
			}
		}
	}

	bool ObjectPicker::DrawPopup(UUID* sceneValue, UUID* objectValue)
	{
		bool objectChosen = false;

		if (ImGui::BeginPopup("ObjectPicker"))
		{
			//ImGui::Text("Asset Picker");
			ImGui::TextUnformatted(ICON_FA_MAGNIFYING_GLASS);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputText("##search", m_FilterBuffer, 200);

			if (m_Filter != std::string(m_FilterBuffer))
			{
				m_Filter = std::string(m_FilterBuffer);
				RefreshFilter();
			}

			if (ImGui::MenuItem("Noone"))
			{
				*sceneValue = 0;
				*objectValue = 0;
				objectChosen = true;
			}

			GScene* pLastScene = nullptr;
			for (size_t i = 0; i < m_FilteredObjects.size(); ++i)
			{
				Entity entity = m_FilteredObjects[i];
				if (pLastScene != entity.GetScene())
				{
					if (pLastScene) ImGui::PopID();
					pLastScene = entity.GetScene();
					ImGui::PushID(pLastScene->Name().c_str());
					ImGui::Separator();
					ImGui::TextDisabled("Scene: %s", pLastScene->Name().c_str());
				}

				const std::string uuidString = std::to_string(entity.EntityUUID());
				ImGui::PushID(uuidString.data());
				const bool selected = *sceneValue == pLastScene->GetUUID() && *objectValue == entity.EntityUUID();
				if (ImGui::Selectable("##selectable", selected, ImGuiSelectableFlags_AllowItemOverlap))
				{
					*sceneValue = pLastScene->GetUUID();
					*objectValue = entity.EntityUUID();
					m_FilteredObjects.clear();
					objectChosen = true;
				}
				ImGui::SameLine();
				ImGui::Text("%s: %s", entity.Name().data(), uuidString.data());
				ImGui::PopID();
			}
			if (pLastScene) ImGui::PopID();

			ImGui::EndPopup();
		}

		if (objectChosen)
		{
			m_Filter = "";
			m_FilteredObjects.clear();
			ImGui::CloseCurrentPopup();
		}
		return objectChosen;
	}
}