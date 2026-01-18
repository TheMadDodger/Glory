#include "FSMNodeEditor.h"
#include "FSMEditor.h"

#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>
#include <Shortcuts.h>

#include <Engine.h>
#include <WindowModule.h>
#include <FSM.h>
#include <YAML_GLM.h>

namespace Glory::Editor
{
	glm::vec2 TempDraggingPos{0.0f};
	UUID DraggingNode{0};
	UUID ConnectingNodeID{0};
	UUID CurrentHoveringNodeID{0};
	bool DraggingView = false;

	FSMNodeEditor::FSMNodeEditor() : EditorWindowTemplate("Nodes", 600.0f, 600.0f), m_PanPosition(0.0f, 0.0f, 0.0f), m_Zoom(1.0f, 1.0f, 1.0f)
	{
	}

	FSMNodeEditor::~FSMNodeEditor()
	{
	}

	void FSMNodeEditor::OnGUI()
	{
		const UUID fsmID = GetMainWindow()->CurrentFSMID();
		if (!fsmID) return;

		UUID& selectedNode = GetMainWindow()->SelectedNode();
		const UUID& activeNode = GetMainWindow()->ActiveNode();
		const UUID& lastActiveNode = GetMainWindow()->LastActiveNode();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(fsmID);
		if (!pResource) return;
		YAMLResource<FSMData>* pDocument = static_cast<YAMLResource<FSMData>*>(pResource);
		Utils::YAMLFileRef& file = pDocument->File();
		Utils::NodeValueRef rootNode = **pDocument;

		const bool allowViewInput = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

		if (allowViewInput && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
		{
			ImGui::FocusWindow(ImGui::GetCurrentWindow());
			m_IsFocused = true;
		}

		const bool allowInput = allowViewInput && !EditorApplication::GetInstance()->IsInPlayMode();

		const ImVec2 windowSize = ImGui::GetWindowSize();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		const glm::vec3 windowOffset{ windowPos.x + windowSize.x/2.0f, windowPos.y + windowSize.y/2.0f, 0.0f };
		const glm::mat4 matOffset = glm::translate(glm::identity<glm::mat4>(), windowOffset);
		const glm::mat4 matPanning = glm::translate(glm::identity<glm::mat4>(), m_PanPosition);
		const glm::mat4 matScaling = glm::scale(glm::identity<glm::mat4>(), m_Zoom);
		const glm::mat4 matFinal = matOffset*matScaling*matPanning;

		auto entryNode = rootNode["StartNode"];
		auto nodes = rootNode["Nodes"];
		auto transitions = rootNode["Transitions"];

		const UUID entryNodeID = entryNode.As<uint64_t>();

		ImDrawList* drawlist = ImGui::GetWindowDrawList();

		ImFont* font = ImGui::GetFont();
		const float rounding = m_Zoom.x*5.0f;
		const float fontSize = m_Zoom.x*ImGui::GetFontSize();
		static const ImU32 defaultColor = ImGui::GetColorU32({ 0.5f, 0.5f, 0.5f, 1.0f });
		static const ImU32 entryNodeColor = ImGui::GetColorU32({ 1.0f, 0.0f, 0.0f, 1.0f });
		static const ImU32 activeNodeColor = ImGui::GetColorU32({ 0.0f, 1.0f, 0.0f, 1.0f });
		static const ImU32 lastActiveNodeColor = ImGui::GetColorU32({ 0.0f, 0.0f, 1.0f, 1.0f });
		static const ImU32 defaultBorderColor = ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 1.0f });
		static const ImU32 selectedBorderColor = ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f });
		bool change = false;

		for (float y = 0; y < windowSize.y; y += 10.0f*m_Zoom.x)
		{
			drawlist->AddLine({ windowPos.x, windowPos.y + y }, { windowPos.x + windowSize.x, windowPos.y + y }, ImGui::GetColorU32({ 0.3f, 0.3f, 0.3f, 0.5f }));
		}

		for (float x = 0; x < windowSize.x; x += 10.0f*m_Zoom.x)
		{
			drawlist->AddLine({ windowPos.x + x, windowPos.y }, { windowPos.x + x, windowPos.y + windowSize.y }, ImGui::GetColorU32({ 0.3f, 0.3f, 0.3f, 0.5f }));
		}

		UUID toRemoveNode = 0;
		CurrentHoveringNodeID = 0;

		bool isMouseInNode = false;
		std::map<UUID, std::pair<glm::vec4, glm::vec2>> cachedNodePositions;
		for (auto iter = nodes.Begin(); iter != nodes.End(); ++iter)
		{
			const std::string key = *iter;
			auto node = nodes[key];
			auto name = node["Name"];
			auto id = node["ID"];
			auto position = node["Position"];
			const std::string nameStr = name.As<std::string>();
			const UUID nodeID = id.As<uint64_t>();
			glm::vec2 pos = DraggingNode == nodeID ? TempDraggingPos : position.As<glm::vec2>({ 0.0f, 0.0f });
			const bool isEntryNode = entryNodeID == nodeID;
			const bool isActiveNode = activeNode == nodeID;
			const bool isLastActiveNode = lastActiveNode == nodeID;
			const bool isSelected = selectedNode == nodeID;

			const ImVec2 textSize = ImGui::CalcTextSize(nameStr.data());
			const glm::vec2 size = { std::max(100.0f, textSize.x), 50.0f };

			const glm::vec4 midPoint = { pos.x, pos.y, 0.0f, 1.0f };
			const glm::vec4 textStartPoint = { midPoint.x - textSize.x/2.0f, midPoint.y - textSize.y/2.0f, 0.0f, 1.0f };
			const glm::vec4 minPoint = { pos.x - size.x/2.0f, pos.y - size.y/2.0f, 0.0f, 1.0f };
			const glm::vec4 maxPoint = { pos.x + size.x/2.0f, pos.y + size.y/2.0f, 0.0f, 1.0f };

			cachedNodePositions.emplace(nodeID, std::pair<glm::vec4, glm::vec2>{ glm::vec4{ minPoint.x, minPoint.y, maxPoint.x, maxPoint.y }, glm::vec2(midPoint) });

			const glm::vec4 transformedTextStartPoint = matFinal*textStartPoint;
			const glm::vec4 transformedMin = matFinal*minPoint;
			const glm::vec4 transformedMax = matFinal*maxPoint;

			drawlist->AddRectFilled({ transformedMin.x, transformedMin.y }, { transformedMax.x, transformedMax.y },
				isActiveNode ? activeNodeColor : isLastActiveNode ? lastActiveNodeColor : isEntryNode ? entryNodeColor : defaultColor, rounding);
			drawlist->AddRect({ transformedMin.x, transformedMin.y }, { transformedMax.x, transformedMax.y }, isSelected ? selectedBorderColor : defaultBorderColor, rounding, 0, isSelected ? 4.0f : 1.0f);
			drawlist->AddText(font, fontSize, { transformedTextStartPoint.x, transformedTextStartPoint.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }), nameStr.data());

			const bool isHovering = ImGui::IsMouseHoveringRect({ transformedMin.x, transformedMin.y }, { transformedMax.x, transformedMax.y });
			isMouseInNode |= isHovering;

			ImGui::PushID(key.data());
			if (DraggingNode == nodeID || isHovering)
			{
				if (allowInput && ConnectingNodeID == 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					if (DraggingNode == 0)
					{
						DraggingNode = nodeID;
						TempDraggingPos = pos;
					}
					else if (DraggingNode == nodeID)
					{
						const ImVec2 panningDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
						TempDraggingPos += glm::vec2{ panningDelta.x, panningDelta.y }*1.0f/m_Zoom.x;
						ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
					}
				}
				else if(DraggingNode == nodeID)
				{
					DraggingNode = 0;
					const glm::vec2 originalPos = position.As<glm::vec2>({ 0.0f, 0.0f });
					Undo::StartRecord("Move Node");
					Undo::ApplyYAMLEdit(file, position.Path(), originalPos, TempDraggingPos);
					Undo::StopRecord();
					change = true;
				}

				CurrentHoveringNodeID = nodeID;

				if (allowViewInput && ConnectingNodeID == 0 && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					selectedNode = nodeID;
				}

				if (allowViewInput && ConnectingNodeID == 0 && DraggingNode == 0 && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					selectedNode = nodeID;
					ImGui::OpenPopup("NodeRightClick");
				}
			}

			ImGui::BeginDisabled(!allowInput);
			if (ImGui::BeginPopup("NodeRightClick"))
			{
				if (ImGui::MenuItem("Set as start node", "", false, entryNodeID != nodeID))
				{
					Undo::StartRecord("Set Start Node");
					Undo::ApplyYAMLEdit(file, entryNode.Path(), uint64_t(entryNodeID), uint64_t(nodeID));
					Undo::StopRecord();
				}
				if (ImGui::MenuItem("Delete", Shortcuts::GetShortcutString("Delete").data(), false))
				{
					toRemoveNode = nodeID;
				}
				if (ImGui::MenuItem("Create Transition", "", false))
				{
					ConnectingNodeID = nodeID;
				}
				ImGui::EndPopup();
			}
			ImGui::EndDisabled();
			ImGui::PopID();
		}

		if (toRemoveNode != 0)
			GetMainWindow()->DeleteNode(toRemoveNode);

		for (auto iter = transitions.Begin(); iter != transitions.End(); ++iter)
		{
			const std::string key = *iter;
			auto transition = transitions[key];
			auto name = transition["Name"];
			auto id = transition["ID"];
			auto from = transition["From"];
			auto to = transition["To"];

			const UUID startNodeID = from.As<uint64_t>();
			const UUID endNodeID = to.As<uint64_t>();

			const auto& startIter = cachedNodePositions.find(startNodeID);
			const auto& endIter = cachedNodePositions.find(endNodeID);
			if (startIter == cachedNodePositions.end() || endIter == cachedNodePositions.end())
				continue;

			const std::pair<glm::vec4, glm::vec2>& startNode = startIter->second;
			const std::pair<glm::vec4, glm::vec2>& endNode = endIter->second;

			const glm::vec4& startRect = startNode.first;
			const glm::vec4& endRect = endNode.first;
			const glm::vec2& startMid = startNode.second;
			const glm::vec2& endMid = endNode.second;

			glm::vec4 start{ startMid.x, startMid.y, 0.0f, 1.0f };
			glm::vec4 end{ endMid.x, endMid.y, 0.0f, 1.0f };

			if (startRect.z < endRect.x)
			{
				/* Left edge */
				end.x = endRect.x;
				end.y = endMid.y;

				/* Right edge */
				start.x = startRect.z;
				start.y = startMid.y;
			}
			else if(startRect.x > endRect.z)
			{
				/* Right edge */
				end.x = endRect.z;
				end.y = endMid.y;

				/* Left edge */
				start.x = startRect.x;
				start.y = startMid.y;
			}
			else if (startRect.w < endRect.y)
			{
				/* Top edge */
				end.x = endMid.x;
				end.y = endRect.y;

				/* Bottom edge */
				start.x = startMid.x;
				start.y = startRect.w;
			}
			else if(startRect.y > endRect.w)
			{
				/* Bottom edge */
				end.x = endMid.x;
				end.y = endRect.w;

				/* Top edge */
				start.x = startMid.x;
				start.y = startRect.y;
			}

			const glm::vec2 startToEnd = glm::vec2(end - start);
			const glm::vec4 middle = { glm::vec2(start) + startToEnd/2.0f, glm::vec2{0.0f, 1.0f} };
			const glm::vec2 offsettedMiddle{ glm::vec2(middle) - glm::normalize(startToEnd)*5.0f };
			const glm::vec3 perpendicular = glm::cross(glm::vec3(glm::normalize(startToEnd), 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			const glm::vec4 trianglePointLeft = { offsettedMiddle - glm::vec2(perpendicular)*5.0f, glm::vec2{0.0f, 1.0f} };
			const glm::vec4 trianglePointRight = { offsettedMiddle + glm::vec2(perpendicular)*5.0f, glm::vec2{0.0f, 1.0f} };

			const glm::vec4 transformedStart = matFinal*start;
			const glm::vec4 transformedEnd = matFinal*end;
			const glm::vec4 transformedMiddle = matFinal*middle;
			const glm::vec4 transformedLeft = matFinal*trianglePointLeft;
			const glm::vec4 transformedRight = matFinal*trianglePointRight;
			ImGui::PushID(key.data());
			drawlist->AddLine({ transformedStart.x, transformedStart.y }, { transformedEnd.x, transformedEnd.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
			drawlist->AddTriangleFilled({ transformedMiddle.x, transformedMiddle.y }, { transformedLeft.x, transformedLeft.y }, { transformedRight.x, transformedRight.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
			ImGui::PopID();
		}

		const ImVec2 mousePos = ImGui::GetMousePos();
		const glm::vec4 transformedMouse = glm::inverse(matFinal)*glm::vec4{ mousePos.x, mousePos.y, 0.0f, 1.0f };
		if (ConnectingNodeID != 0 && allowInput)
		{
			const auto& startIter = cachedNodePositions.find(ConnectingNodeID);
			const std::pair<glm::vec4, glm::vec2>& startNode = startIter->second;

			glm::vec4 endRect = { transformedMouse.x, transformedMouse.y, transformedMouse.x, transformedMouse.y };
			glm::vec2 endMid{ transformedMouse.x, transformedMouse.y };

			if (CurrentHoveringNodeID != 0)
			{
				const auto& endIter = cachedNodePositions.find(CurrentHoveringNodeID);
				const std::pair<glm::vec4, glm::vec2>& endNode = endIter->second;
				endRect = endNode.first;
				endMid = endNode.second;
			}

			const glm::vec4& startRect = startNode.first;
			const glm::vec2& startMid = startNode.second;

			glm::vec4 start{ startMid, glm::vec2{0.0f, 1.0f} };
			glm::vec4 end{ glm::vec2{ transformedMouse.x, transformedMouse.y }, glm::vec2{0.0f, 1.0f} };

			if (startRect.z < endRect.x)
			{
				/* Left edge */
				end.x = endRect.x;
				end.y = endMid.y;

				/* Right edge */
				start.x = startRect.z;
				start.y = startMid.y;
			}
			else if (startRect.x > endRect.z)
			{
				/* Right edge */
				end.x = endRect.z;
				end.y = endMid.y;

				/* Left edge */
				start.x = startRect.x;
				start.y = startMid.y;
			}
			else if (startRect.w < endRect.y)
			{
				/* Top edge */
				end.x = endMid.x;
				end.y = endRect.y;

				/* Bottom edge */
				start.x = startMid.x;
				start.y = startRect.w;
			}
			else if (startRect.y > endRect.w)
			{
				/* Bottom edge */
				end.x = endMid.x;
				end.y = endRect.w;

				/* Top edge */
				start.x = startMid.x;
				start.y = startRect.y;
			}

			const glm::vec2 startToEnd = glm::vec2(end - start);
			const glm::vec4 middle = { glm::vec2(start) + startToEnd / 2.0f, glm::vec2{0.0f, 1.0f} };
			const glm::vec2 offsettedMiddle{ glm::vec2(middle) - glm::normalize(startToEnd) * 5.0f };
			const glm::vec3 perpendicular = glm::cross(glm::vec3(glm::normalize(startToEnd), 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			const glm::vec4 trianglePointLeft = { offsettedMiddle - glm::vec2(perpendicular) * 5.0f, glm::vec2{0.0f, 1.0f} };
			const glm::vec4 trianglePointRight = { offsettedMiddle + glm::vec2(perpendicular) * 5.0f, glm::vec2{0.0f, 1.0f} };

			const glm::vec4 transformedStart = matFinal*start;
			const glm::vec4 transformedEnd = matFinal*end;
			const glm::vec4 transformedMiddle = matFinal*middle;
			const glm::vec4 transformedLeft = matFinal*trianglePointLeft;
			const glm::vec4 transformedRight = matFinal*trianglePointRight;
			drawlist->AddLine({ transformedStart.x, transformedStart.y }, { transformedEnd.x, transformedEnd.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
			drawlist->AddTriangleFilled({ transformedMiddle.x, transformedMiddle.y }, { transformedLeft.x, transformedLeft.y }, { transformedRight.x, transformedRight.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));

			if (allowViewInput && CurrentHoveringNodeID != ConnectingNodeID && CurrentHoveringNodeID != 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				const UUID newTransitionID{};
				const std::string idStr = std::to_string(newTransitionID);
				auto transition = transitions[idStr];

				YAML::Node newTransitionNode{ YAML::NodeType::Map };
				newTransitionNode["ID"] = uint64_t(newTransitionID);
				newTransitionNode["Name"] = "New Transition";
				newTransitionNode["From"] = uint64_t(ConnectingNodeID);
				newTransitionNode["To"] = uint64_t(CurrentHoveringNodeID);
				newTransitionNode["Property"] = uint64_t(0);
				newTransitionNode["OP"] = "Custom";
				newTransitionNode["CompareValue"] = 0;

				Undo::StartRecord("New Transition");
				Undo::YAMLEdit(file, transition.Path(), YAML::Node{ YAML::NodeType::Null }, newTransitionNode);
				Undo::StopRecord();

				ConnectingNodeID = 0;
				CurrentHoveringNodeID = 0;
			}
			else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				ConnectingNodeID = 0;
			}
		}
		else if (!allowInput)
		{
			ConnectingNodeID = 0;
			CurrentHoveringNodeID = 0;
		}

		if (allowViewInput && !DraggingView && ImGui::IsMouseReleased(ImGuiMouseButton_Right)
			&& ConnectingNodeID == 0 && DraggingNode == 0 && CurrentHoveringNodeID == 0)
		{
			ImGui::OpenPopup("RightClick");
		}

		ImGui::BeginDisabled(!allowInput);
		if (ImGui::BeginPopup("RightClick"))
		{
			if (ImGui::MenuItem("New Node", "", false))
			{
				const UUID newNodeID{};
				const std::string idStr = std::to_string(newNodeID);
				auto node = nodes[idStr];

				YAML::Node newNodeNode{ YAML::NodeType::Map };
				newNodeNode["ID"] = uint64_t(newNodeID);
				newNodeNode["Name"] = "New Node";
				newNodeNode["Position"] = glm::vec2(transformedMouse.x, transformedMouse.y);

				Undo::StartRecord("New Transition");
				Undo::YAMLEdit(file, node.Path(), YAML::Node{ YAML::NodeType::Null }, newNodeNode);
				Undo::StopRecord();
			}
			ImGui::EndPopup();
		}
		ImGui::EndDisabled();

		if (allowViewInput)
		{
			const ImVec2 panningDelta = ImGui::IsMouseDown(ImGuiMouseButton_Middle) ? ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle) :
				ImGui::IsMouseDown(ImGuiMouseButton_Right) ? ImGui::GetMouseDragDelta(ImGuiMouseButton_Right) : ImVec2{};
			m_PanPosition.x += panningDelta.x * 1.0f / m_Zoom.x;
			m_PanPosition.y += panningDelta.y * 1.0f / m_Zoom.x;
			DraggingView |= panningDelta.x != 0.0f || panningDelta.y != 0.0f;
			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

			m_Zoom += ImGui::GetIO().MouseWheel * 0.1f;
			m_Zoom = glm::clamp(m_Zoom, glm::vec3{ 0.5f }, glm::vec3{ 10.0f });
		}

		if ((DraggingView && !ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !ImGui::IsMouseDown(ImGuiMouseButton_Right)) || !allowViewInput)
			DraggingView = false;

		if (allowViewInput && !isMouseInNode && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			selectedNode = 0;
		}

		if (change)
			EditorAssetDatabase::SetAssetDirty(fsmID);
	}

	void FSMNodeEditor::Update()
	{
	}

	void FSMNodeEditor::Draw()
	{
	}

	FSMEditor* FSMNodeEditor::GetMainWindow()
	{
		return static_cast<FSMEditor*>(m_pOwner);
	}
}
