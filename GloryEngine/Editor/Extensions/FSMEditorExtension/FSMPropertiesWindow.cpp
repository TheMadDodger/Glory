#include "FSMPropertiesWindow.h"
#include "FSMEditor.h"

#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>

#include <Engine.h>
#include <FSM.h>

#include <StringUtils.h>

namespace Glory::Editor
{
	char SearchBuffer[256] = "\0";
	bool ForceFilter = false;

	FSMPropertiesWindow::FSMPropertiesWindow() : EditorWindowTemplate("Properties", 600.0f, 600.0f), m_LastFrameFSMID(0)
	{
		m_WindowFlags = ImGuiWindowFlags_MenuBar;
	}

	FSMPropertiesWindow::~FSMPropertiesWindow()
	{
	}

	void FSMPropertiesWindow::OnGUI()
	{
		const UUID fsmID = GetMainWindow()->CurrentFSMID();
		if (!fsmID) return;

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(fsmID);
		if (!pResource) return;
		YAMLResource<FSMData>* pDocument = static_cast<YAMLResource<FSMData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocument;

		auto properties = file["Properties"];

		bool change = false;
		bool needsFilter = m_LastFrameFSMID != fsmID || ForceFilter;
		m_LastFrameFSMID = fsmID;

		if (ImGui::BeginMenuBar())
		{
			const float searchBarWidth = ImGui::GetContentRegionAvail().x - 100.0f;
			needsFilter |= EditorUI::SearchBar(searchBarWidth, SearchBuffer, 256);

			if (ImGui::BeginMenu("Add Property"))
			{
				auto enumType = Enum<FSMPropertyType>();
				for (size_t i = 0; i < enumType.NumValues(); ++i)
				{
					const std::string& name = enumType.GetName(i);
					if (ImGui::MenuItem(name.data()))
					{
						UUID newPropID{};
						const std::string idStr = std::to_string(newPropID);
						auto prop = properties[idStr];

						YAML::Node newNode{ YAML::NodeType::Map };
						newNode["ID"] = uint64_t(newPropID);
						newNode["Type"] = name;
						newNode["Name"] = "New Property";

						Undo::StartRecord("Add Property");
						Undo::YAMLEdit(file, prop.Path(), YAML::Node(YAML::NodeType::Null), newNode);
						Undo::StopRecord();
					}
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		static std::vector<UUID> filteredProperties;

		if (needsFilter)
		{
			filteredProperties.clear();
			const std::string_view search = SearchBuffer;

			for (auto iter = properties.Begin(); iter != properties.End(); ++iter)
			{
				const std::string key = *iter;
				auto prop = properties[key];
				auto name = prop["Name"];
				auto id = prop["ID"];
				const std::string nameStr = name.As<std::string>();
				const UUID propID = id.As<uint64_t>();

				if (search.empty() || Utils::CaseInsensitiveSearch(nameStr, search) != std::string::npos)
					filteredProperties.push_back(propID);
			}
		}

		std::string toRemoveProp = "";
		for (size_t i = 0; i < filteredProperties.size(); ++i)
		{
			const UUID propID = filteredProperties[i];
			const std::string key = std::to_string(propID);
			auto prop = properties[key];
			auto type = prop["Type"];
			auto name = prop["Name"];

			ImGui::PushID(key.data());
			ImGui::BeginChild(key.data(), ImVec2{0.0f, 68.0f}, true, ImGuiWindowFlags_AlwaysAutoResize);
			change |= EditorUI::InputText(file, name.Path());
			change |= EditorUI::InputEnum<FSMPropertyType>(file, type.Path());

			if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				ImGui::OpenPopup("PropertyRightClick");

			if (ImGui::BeginPopup("PropertyRightClick"))
			{
				if (ImGui::MenuItem("Remove", "", false))
					toRemoveProp = key;
				ImGui::EndPopup();
			}

			ImGui::EndChild();
			ImGui::PopID();
		}

		if (!toRemoveProp.empty())
		{
			auto prop = properties[toRemoveProp];
			Undo::StartRecord("Remove Property");
			Undo::YAMLEdit(file, prop.Path(), prop.Node(), YAML::Node(YAML::NodeType::Null));
			Undo::StopRecord();
			ForceFilter = true;
		}

		if (change)
			EditorAssetDatabase::SetAssetDirty(fsmID);
	}

	void FSMPropertiesWindow::Update()
	{
	}

	void FSMPropertiesWindow::Draw()
	{
	}

	FSMEditor* FSMPropertiesWindow::GetMainWindow()
	{
		return static_cast<FSMEditor*>(m_pOwner);
	}
}
