#include "FSMNodeInspector.h"
#include "FSMEditor.h"

#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>

#include <Engine.h>
#include <WindowModule.h>
#include <FSM.h>

#include <StringUtils.h>

namespace Glory::Editor
{
	bool ForceTransitionFilter = false;
	bool ForcePropertiesFilter = false;
	std::vector<UUID> CachedFromTransitions;
	std::vector<UUID> CachedToTransitions;

	FSMNodeInspector::FSMNodeInspector() : EditorWindowTemplate("Inspector", 600.0f, 600.0f), m_LastFrameSelectedNode(0)
	{
		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("Transitions"),
		[](Utils::YAMLFileRef&, const std::filesystem::path&) {
			ForceTransitionFilter = true;
		});

		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("Properties"),
		[](Utils::YAMLFileRef&, const std::filesystem::path&) {
			ForcePropertiesFilter = true;
		});
	}

	FSMNodeInspector::~FSMNodeInspector()
	{
	}

	void FSMNodeInspector::OnGUI()
	{
		const UUID fsmID = GetMainWindow()->CurrentFSMID();
		UUID& selectedNode = GetMainWindow()->SelectedNode();
		if (!fsmID)
		{
			CachedFromTransitions.clear();
			CachedToTransitions.clear();
			m_LastFrameSelectedNode = 0;
			return;
		}

		if (!selectedNode)
		{
			CachedFromTransitions.clear();
			CachedToTransitions.clear();
			m_LastFrameSelectedNode = 0;
			ImGui::TextUnformatted("Select a node to edit it here.");
			return;
		}

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(fsmID);
		if (!pResource) return;
		YAMLResource<FSMData>* pDocument = static_cast<YAMLResource<FSMData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocument;

		auto nodes = file["Nodes"];
		auto transitions = file["Transitions"];
		auto properties = file["Properties"];

		const bool needsFilter = m_LastFrameSelectedNode != selectedNode || ForceTransitionFilter;
		m_LastFrameSelectedNode = selectedNode;
		ForceTransitionFilter = false;

		if (needsFilter)
		{
			CachedFromTransitions.clear();
			CachedToTransitions.clear();

			for (auto iter = transitions.Begin(); iter != transitions.End(); ++iter)
			{
				auto transition = transitions[*iter];
				const UUID id = transition["ID"].As<uint64_t>();
				const UUID from = transition["From"].As<uint64_t>();
				const UUID to = transition["To"].As<uint64_t>();

				if (from == selectedNode)
					CachedFromTransitions.emplace_back(id);
				if (to == selectedNode)
					CachedToTransitions.emplace_back(id);
			}
		}

		const bool inPlayMode = EditorApplication::GetInstance()->IsInPlayMode();
		const std::string idStr = std::to_string(selectedNode);
		auto node = nodes[idStr];
		ImGui::PushID(idStr.data());
		EditorUI::LabelText("Node ID", idStr);
		ImGui::BeginDisabled(inPlayMode);
		bool change = EditorUI::InputText(file, node["Name"].Path());
		ImGui::EndDisabled();

		ImGui::TextUnformatted("Transitions from this node");
		for (size_t i = 0; i < CachedFromTransitions.size(); ++i)
		{
			const UUID transitionID = CachedFromTransitions[i];
			const std::string transIDStr = std::to_string(transitionID);
			auto transition = transitions[transIDStr];
			auto name = transition["Name"];
			auto transitionProp = transition["Property"];
			UUID propID = transitionProp.As<uint64_t>(0);
			const UUID oldPropID = propID;
			auto op = transition["OP"];
			auto compareValue = transition["CompareValue"];
			const UUID to = transition["To"].As<uint64_t>();
			const std::string toIDStr = std::to_string(to);
			auto toNode = nodes[toIDStr];
			FSMPropertyType propertyType = FSMPropertyType::Trigger;

			std::string currentPropName = "None";
			if (propID)
			{
				const std::string propIDStr = std::to_string(propID);
				auto prop = properties[propIDStr];
				currentPropName = prop["Name"].As<std::string>();
				propertyType = prop["Type"].AsEnum<FSMPropertyType>();
			}

			bool propSet = false;
			ImGui::PushID(toIDStr.data());
			if (EditorUI::Header(name.As<std::string>()))
			{
				ImGui::BeginDisabled(inPlayMode);
				change |= EditorUI::InputText(file, name.Path());
				ImGui::EndDisabled();
				ImGui::Text("To: %s", toNode["Name"].As<std::string>().data());
				ImGui::SameLine();
				constexpr float buttonWidth = 50.0f;
				const float availableWidth = ImGui::GetContentRegionAvail().x;
				const float cursorX = ImGui::GetCursorPosX();
				ImGui::SetCursorPosX(cursorX + availableWidth - buttonWidth - ImGui::GetStyle().FramePadding.x);
				if (ImGui::Button("Jump", { buttonWidth , 0.0f }))
					selectedNode = to;

				float start, width;
				bool openPopup = false;
				ImGui::BeginDisabled(inPlayMode);
				EditorUI::EmptyDropdown("Property", currentPropName, [&openPopup]() {
					openPopup = true;
					ForcePropertiesFilter = true;
				}, start, width);

				FSMTransitionOP transitionOP = op.AsEnum<FSMTransitionOP>(FSMTransitionOP::Trigger);
				bool disable = false;
				std::vector<uint32_t> excludedOptions;
				if (propID == 0)
				{
					disable = true;
					if (transitionOP != FSMTransitionOP::Custom)
						Undo::ApplyYAMLEdit(file, op.Path(), op.As<std::string>(), std::string("Custom"));
				}
				else
				{
					switch (propertyType)
					{
					case Glory::FSMPropertyType::Number:
						excludedOptions.push_back(uint32_t(FSMTransitionOP::Trigger));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::On));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::Off));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::Custom));
						if (transitionOP < FSMTransitionOP::Equal || transitionOP > FSMTransitionOP::LessOrEqual)
							Undo::ApplyYAMLEdit(file, op.Path(), op.As<std::string>(), std::string("Equal"));
						break;
					case Glory::FSMPropertyType::Bool:
						excludedOptions.push_back(uint32_t(FSMTransitionOP::Trigger));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::Equal));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::Greater));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::GreaterOrEqual));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::Less));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::LessOrEqual));
						excludedOptions.push_back(uint32_t(FSMTransitionOP::Custom));
						if (transitionOP != FSMTransitionOP::On && transitionOP != FSMTransitionOP::Off)
							Undo::ApplyYAMLEdit(file, op.Path(), op.As<std::string>(), std::string("On"));
						break;
					case Glory::FSMPropertyType::Trigger:
						disable = true;
						if (transitionOP != FSMTransitionOP::Trigger)
							Undo::ApplyYAMLEdit(file, op.Path(), op.As<std::string>(), std::string("Trigger"));
						break;
					default:
						break;
					}
				}
				ImGui::EndDisabled();

				ImGui::BeginDisabled(inPlayMode || disable);
				change |= EditorUI::InputEnum<FSMTransitionOP>(file, op.Path(), excludedOptions);
				ImGui::EndDisabled();
				transitionOP = op.AsEnum<FSMTransitionOP>(FSMTransitionOP::Trigger);

				switch (transitionOP)
				{
				case FSMTransitionOP::Equal:
				case FSMTransitionOP::Greater:
				case FSMTransitionOP::GreaterOrEqual:
				case FSMTransitionOP::Less:
				case FSMTransitionOP::LessOrEqual:
					change |= EditorUI::InputFloat(file, compareValue.Path());
					break;
				}

				if (openPopup)
					ImGui::OpenPopup("PropertiesDropdown");

				const ImVec2 cursor = ImGui::GetCursorPos();
				const ImVec2 windowPos = ImGui::GetWindowPos();
				Window* pWindow = EditorApplication::GetInstance()->GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
				int mainWindowWidth, mainWindowHeight;
				pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
				ImGui::SetNextWindowPos({ windowPos.x + start, windowPos.y + cursor.y - 2.5f });
				ImGui::SetNextWindowSize({ width, mainWindowHeight - windowPos.y - cursor.y - 10.0f });
				if (ImGui::BeginPopup("PropertiesDropdown"))
				{
					static char searchBuffer[200] = "";
					static std::vector<UUID> searchResultCache;

					const bool needsFilter = EditorUI::SearchBar(ImGui::GetContentRegionAvail().x, searchBuffer, 200) || ForcePropertiesFilter;
					ForcePropertiesFilter = false;
					if (needsFilter)
					{
						searchResultCache.clear();
						const std::string_view search = searchBuffer;

						for (auto iter = properties.Begin(); iter != properties.End(); ++iter)
						{
							const std::string key = *iter;
							auto prop = properties[key];
							auto name = prop["Name"];
							auto id = prop["ID"];
							const std::string nameStr = name.As<std::string>();
							const UUID propID = id.As<uint64_t>();

							if (search.empty() || Utils::CaseInsensitiveSearch(nameStr, search) != std::string::npos)
								searchResultCache.push_back(propID);
						}
					}

					if (ImGui::Selectable("None", propID == 0))
					{
						propID = 0;
						propSet = true;
					}

					ImGui::BeginChild("scrollregion");
					const float rowHeight = 22.0f;
					ImGuiListClipper clipper{ int(searchResultCache.size()), rowHeight };

					auto itorStart = searchResultCache.begin();
					while (clipper.Step()) {
						const auto start = itorStart + clipper.DisplayStart;
						const auto end = itorStart + clipper.DisplayEnd;

						for (auto it = start; it != end; ++it)
						{
							const UUID otherPropID = *it;
							const std::string otherPropIDStr = std::to_string(otherPropID);
							ImGui::PushID(otherPropIDStr.data());
							auto oltherProp = properties[otherPropIDStr];
							const std::string name = oltherProp["Name"].As<std::string>();
							if (ImGui::Selectable("##select", propID == otherPropID, ImGuiSelectableFlags_AllowItemOverlap, { 0.0f, rowHeight }))
							{
								propID = otherPropID;
								propSet = true;
							}

							ImGui::SameLine();
							ImGui::TextUnformatted(name.data());
							ImGui::PopID();
						}
					}
					ImGui::EndChild();

					if (propSet)
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}
			}

			if (propSet)
			{
				Undo::StartRecord("Change Property");
				Undo::ApplyYAMLEdit(file, transitionProp.Path(), uint64_t(oldPropID), uint64_t(propID));
				const FSMTransitionOP transitionOP = op.AsEnum<FSMTransitionOP>(FSMTransitionOP::Trigger);
				const std::string transitionOPStr = op.As<std::string>("Trigger");

				/* Enforce compare OP */
				if (!propID)
				{
					/* Force to custom */
					if (transitionOP != FSMTransitionOP::Custom)
						Undo::ApplyYAMLEdit(file, op.Path(), transitionOPStr, std::string("Custom"));
				}
				else
				{
					const std::string newPropIDStr = std::to_string(propID);
					auto newProp = properties[newPropIDStr];
					const FSMPropertyType type = newProp["Type"].AsEnum<FSMPropertyType>();

					switch (type)
					{
					case FSMPropertyType::Number:
						if (transitionOP < FSMTransitionOP::Equal || transitionOP > FSMTransitionOP::LessOrEqual)
						{
							/* Force Equal */
							Undo::ApplyYAMLEdit(file, op.Path(), transitionOPStr, std::string("Equal"));
						}
						break;
					case FSMPropertyType::Bool:
						if (transitionOP != FSMTransitionOP::On && transitionOP != FSMTransitionOP::Off)
						{
							/* Force On */
							Undo::ApplyYAMLEdit(file, op.Path(), transitionOPStr, std::string("On"));
						}
						break;
					case FSMPropertyType::Trigger:
						if (transitionOP != FSMTransitionOP::Trigger)
						{
							/* Force trigger */
							Undo::ApplyYAMLEdit(file, op.Path(), transitionOPStr, std::string("Trigger"));
						}
						break;
					default:
						break;
					}
				}

				Undo::StopRecord();
				change = true;
			}
			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Transitions to this node");
		for (size_t i = 0; i < CachedToTransitions.size(); ++i)
		{
			const UUID transitionID = CachedToTransitions[i];
			const std::string transIDStr = std::to_string(transitionID);
			auto transition = transitions[transIDStr];
			auto name = transition["Name"];
			const UUID from = transition["From"].As<uint64_t>();
			const std::string fromIDStr = std::to_string(from);
			auto fromNode = nodes[fromIDStr];

			ImGui::PushID(fromIDStr.data());
			ImGui::Text("From: %s", fromNode["Name"].As<std::string>().data());
			ImGui::SameLine();
			constexpr float buttonWidth = 50.0f;
			const float availableWidth = ImGui::GetContentRegionAvail().x;
			const float cursorX = ImGui::GetCursorPosX();
			ImGui::SetCursorPosX(cursorX + availableWidth - buttonWidth - ImGui::GetStyle().FramePadding.x);
			if (ImGui::Button("Jump", { buttonWidth , 0.0f }))
				selectedNode = from;
			ImGui::PopID();
		}
		ImGui::PopID();
	}

	void FSMNodeInspector::Update()
	{
	}

	void FSMNodeInspector::Draw()
	{
	}

	FSMEditor* FSMNodeInspector::GetMainWindow()
	{
		return static_cast<FSMEditor*>(m_pOwner);
	}
}
