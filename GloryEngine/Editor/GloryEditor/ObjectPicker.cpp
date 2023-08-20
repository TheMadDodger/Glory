#include "ObjectPicker.h"
#include "EditorUI.h"
#include "DND.h"

#include <WindowModule.h>
#include <Engine.h>
#include <imgui.h>
#include <algorithm>
#include <EditorAssetDatabase.h>

#include <IconsFontAwesome6.h>
#include <EditorSceneManager.h>

namespace Glory::Editor
{
	DND DragAndDropTarget = { { ResourceType::GetHash<SceneObject>() }};

	char ObjectPicker::m_FilterBuffer[200] = "";
	std::string ObjectPicker::m_Filter = "";
	std::vector<SceneObject*> ObjectPicker::m_FilteredObjects;

	bool ObjectPicker::ObjectDropdown(const std::string& label, SceneObjectRef* value, const float borderPadding)
	{
		return ObjectDropdown(label, value->SceneUUIDMember(), value->ObjectUUIDMember(), borderPadding);
	}

	bool ObjectPicker::ObjectDropdown(const std::string& label, UUID* sceneValue, UUID* objectValue, const float borderPadding)
	{
		GScene* pScene = EditorSceneManager::GetOpenScene(*sceneValue);
		SceneObject* pObject = pScene ? pScene->FindSceneObject(*objectValue) : nullptr;

		ImGui::PushID(label.c_str());
		std::string objectName = "";
		bool missing = *sceneValue != 0 && *objectValue != 0 && (pScene == nullptr || pObject == nullptr);

		objectName = missing ? "Missing Object" : (!pObject ? "Noone" : pObject->Name() + ": " + std::to_string(pObject->GetUUID()));

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
			*sceneValue = objectPayload.pObject->GetScene()->GetUUID();
			*objectValue = objectPayload.pObject->GetUUID();
		});

		if (openPopup)
		{
			ImGui::OpenPopup("ObjectPicker");
			RefreshFilter();
		}
		openPopup = false;

		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		Window* pWindow = Game::GetGame().GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
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
		for (size_t i = 0; i < EditorSceneManager::OpenSceneCount(); ++i)
		{
			GScene* pScene = EditorSceneManager::GetOpenScene(i);
			for (size_t j = 0; j < pScene->SceneObjectsCount(); ++j)
			{
				SceneObject* pObject = pScene->GetSceneObject(j);
				if (!m_Filter.empty() && pObject->Name().find(m_Filter) == std::string::npos &&
					std::to_string(pObject->GetUUID()).find(m_Filter) == std::string::npos) continue;
				m_FilteredObjects.push_back(pObject);
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
				SceneObject* pObject = m_FilteredObjects[i];
				if (pLastScene != pObject->GetScene())
				{
					if (pLastScene) ImGui::PopID();
					pLastScene = pObject->GetScene();
					ImGui::PushID(pLastScene->Name().c_str());
					ImGui::Separator();
					ImGui::TextDisabled("Scene: %s", pLastScene->Name().c_str());
				}

				const std::string uuidString = std::to_string(pObject->GetUUID());
				ImGui::PushID(uuidString.data());
				const bool selected = *sceneValue == pLastScene->GetUUID() && *objectValue == pObject->GetUUID();
				if (ImGui::Selectable("##selectable", selected, ImGuiSelectableFlags_AllowItemOverlap))
				{
					*sceneValue = pLastScene->GetUUID();
					*objectValue = pObject->GetUUID();
					m_FilteredObjects.clear();
					objectChosen = true;
				}
				ImGui::SameLine();
				ImGui::Text("%s: %s", pObject->Name().c_str(), uuidString.data());
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