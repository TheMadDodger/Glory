#include "FSMPropertiesWindow.h"
#include "FSMEditor.h"

#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>

#include <Engine.h>
#include <FSM.h>
#include <FSMModule.h>
#include <AssetManager.h>

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
		Utils::YAMLFileRef& file = pDocument->File();
		Utils::NodeValueRef rootNode = **pDocument;

		auto properties = rootNode["Properties"];
		auto transitions = rootNode["Transitions"];

		bool change = false;
		bool needsFilter = m_LastFrameFSMID != fsmID || ForceFilter;
		m_LastFrameFSMID = fsmID;

		const bool inPlayMode = EditorApplication::GetInstance()->IsInPlayMode();
		if (ImGui::BeginMenuBar())
		{
			const float searchBarWidth = ImGui::GetContentRegionAvail().x - 100.0f;
			needsFilter |= EditorUI::SearchBar(searchBarWidth, SearchBuffer, 256);

			ImGui::BeginDisabled(inPlayMode);
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
			ImGui::EndDisabled();
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

		UUID& debuggingInstance = GetMainWindow()->DebuggingInstance();
		FSMModule* pFSMModule = pEngine->GetOptionalModule<FSMModule>();
		const FSMState* debuggingState = debuggingInstance ? pFSMModule->FSMInstance(debuggingInstance) : nullptr;
		FSMData* debuggingFSM = nullptr;

		if (debuggingState)
		{
			Resource* pResource = pEngine->GetAssetManager().FindResource(fsmID);
			if (pResource) debuggingFSM = static_cast<FSMData*>(pResource);
		}

		ImGui::BeginDisabled(inPlayMode);
		std::string toRemoveProp = "";
		for (size_t i = 0; i < filteredProperties.size(); ++i)
		{
			const UUID propID = filteredProperties[i];
			const std::string key = std::to_string(propID);
			auto prop = properties[key];
			auto type = prop["Type"];
			auto name = prop["Name"];

			ImGui::PushID(key.data());
			ImGui::BeginChild(key.data(), ImVec2{ 0.0f, inPlayMode && debuggingState && debuggingFSM ? 96.0f : 68.0f },
				true, ImGuiWindowFlags_AlwaysAutoResize);
			change |= EditorUI::InputText(file, name.Path());

			Undo::StartRecord("Change Property");
			const bool changedType = EditorUI::InputEnum<FSMPropertyType>(file, type.Path());

			if (changedType)
			{
				for (auto iter = transitions.Begin(); iter != transitions.End(); ++iter)
				{
					auto transition = transitions[*iter];
					auto transitionProp = transition["Property"];
					const UUID transitionPropID = transitionProp.As<uint64_t>();
					if (transitionPropID != propID) continue;
					auto op = transition["OP"];
					auto transitionOP = op.AsEnum<FSMTransitionOP>();
					
					switch (type.AsEnum<FSMPropertyType>())
					{
					case Glory::FSMPropertyType::Number:
						if (transitionOP < FSMTransitionOP::Equal || transitionOP > FSMTransitionOP::LessOrEqual)
							Undo::ApplyYAMLEdit(file, op.Path(), op.As<std::string>(), std::string("Equal"));
						break;
					case Glory::FSMPropertyType::Bool:
						if (transitionOP != FSMTransitionOP::On && transitionOP != FSMTransitionOP::Off)
							Undo::ApplyYAMLEdit(file, op.Path(), op.As<std::string>(), std::string("On"));
						break;
					case Glory::FSMPropertyType::Trigger:
						if (transitionOP != FSMTransitionOP::Trigger)
							Undo::ApplyYAMLEdit(file, op.Path(), op.As<std::string>(), std::string("Trigger"));
						break;
					default:
						break;
					}
				}
			}

			change |= changedType;
			Undo::StopRecord();

			if (inPlayMode && debuggingState && debuggingFSM)
			{
				const FSMPropertyType propType = type.AsEnum<FSMPropertyType>();
				switch (propType)
				{
				case FSMPropertyType::Trigger:
				{
					int triggerInt;
					debuggingState->GetPropertyValue(debuggingFSM, name.As<std::string>(), &triggerInt);
					bool trigger = triggerInt > 0;
					EditorUI::CheckBox("Trigger", &trigger);
					break;
				}
				case FSMPropertyType::Bool:
				{
					bool boolean;
					debuggingState->GetPropertyValue(debuggingFSM, name.As<std::string>(), &boolean);
					EditorUI::CheckBox("Boolean", &boolean);
					break;
				}
				case FSMPropertyType::Number:
				{
					float number;
					debuggingState->GetPropertyValue(debuggingFSM, name.As<std::string>(), &number);
					EditorUI::InputFloat("Number", &number);
					break;
				}
				}
			}

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
			const UUID propID = prop["ID"].As<uint64_t>();
			Undo::StartRecord("Remove Property");
			Undo::YAMLEdit(file, prop.Path(), prop.Node(), YAML::Node(YAML::NodeType::Null));

			for (auto iter = transitions.Begin(); iter != transitions.End(); ++iter)
			{
				auto transition = transitions[*iter];
				auto transitionProp = transition["Property"];
				const UUID transitionPropID = transitionProp.As<uint64_t>();
				if (transitionPropID != propID) continue;
				auto transitionOP = transition["OP"];
				Undo::ApplyYAMLEdit(file, transitionProp.Path(), uint64_t(transitionPropID), uint64_t(0));
				Undo::ApplyYAMLEdit(file, transitionOP.Path(), transitionOP.As<std::string>(), std::string("Custom"));
			}

			Undo::StopRecord();
			ForceFilter = true;
		}
		ImGui::EndDisabled();

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
