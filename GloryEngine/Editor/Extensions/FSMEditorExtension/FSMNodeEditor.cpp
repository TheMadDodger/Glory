#include "FSMNodeEditor.h"
#include "FSMEditor.h"

#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>

#include <Engine.h>
#include <WindowModule.h>
#include <FSM.h>
#include <YAML_GLM.h>

namespace Glory::Editor
{
	glm::vec2 TempDraggingPos{0.0f};
	UUID DraggingNode{0};

	FSMNodeEditor::FSMNodeEditor() : EditorWindowTemplate("Nodes", 600.0f, 600.0f), m_PanPosition(0.0f, 0.0f, 0.0f), m_Zoom(1.0f, 1.0f, 1.0f)
	{
	}

	FSMNodeEditor::~FSMNodeEditor()
	{
	}

	void FSMNodeEditor::OnGUI()
	{
		const UUID fsmID = GetMainWindow()->CurrentFSMID();
		UUID& selectedNode = GetMainWindow()->SelectedNode();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(fsmID);
		YAMLResource<FSMData>* pDocument = static_cast<YAMLResource<FSMData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocument;

		const ImVec2 windowSize = ImGui::GetWindowSize();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		const glm::vec3 windowOffset{ windowPos.x + windowSize.x/2.0f, windowPos.y + windowSize.y/2.0f, 0.0f };
		const glm::mat4 matOffset = glm::translate(glm::identity<glm::mat4>(), windowOffset);
		const glm::mat4 matPanning = glm::translate(glm::identity<glm::mat4>(), m_PanPosition);
		const glm::mat4 matScaling = glm::scale(glm::identity<glm::mat4>(), m_Zoom);
		const glm::mat4 matFinal = matOffset*matScaling*matPanning;

		auto entryNode = file["StartNode"];
		auto nodes = file["Nodes"];
		auto transitions = file["Transitions"];

		const UUID entryNodeID = entryNode.As<uint64_t>();

		ImDrawList* drawlist = ImGui::GetWindowDrawList();

		ImFont* font = ImGui::GetFont();
		const float rounding = m_Zoom.x*5.0f;
		const float fontSize = m_Zoom.x*ImGui::GetFontSize();
		static const ImU32 defaultColor = ImGui::GetColorU32({ 0.5f, 0.5f, 0.5f, 1.0f });
		static const ImU32 entryNodeColor = ImGui::GetColorU32({ 1.0f, 0.0f, 0.0f, 1.0f });
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

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			selectedNode = 0;
		}

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

			drawlist->AddRectFilled({ transformedMin.x, transformedMin.y }, { transformedMax.x, transformedMax.y }, isEntryNode ? entryNodeColor : defaultColor, rounding);
			drawlist->AddRect({ transformedMin.x, transformedMin.y }, { transformedMax.x, transformedMax.y }, isSelected ? selectedBorderColor : defaultBorderColor, rounding, 0, isSelected ? 4.0f : 1.0f);
			drawlist->AddText(font, fontSize, { transformedTextStartPoint.x, transformedTextStartPoint.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }), nameStr.data());

			if (DraggingNode == nodeID || ImGui::IsMouseHoveringRect({ transformedMin.x, transformedMin.y }, { transformedMax.x, transformedMax.y }))
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
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

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					selectedNode = nodeID;
				}
			}
		}

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
			drawlist->AddLine({ transformedStart.x, transformedStart.y }, { transformedEnd.x, transformedEnd.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
			drawlist->AddTriangleFilled({ transformedMiddle.x, transformedMiddle.y }, { transformedLeft.x, transformedLeft.y }, { transformedRight.x, transformedRight.y }, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
		}

		const ImVec2 panningDelta = ImGui::IsMouseDown(ImGuiMouseButton_Middle) ? ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle) :
			ImGui::IsMouseDown(ImGuiMouseButton_Right) ? ImGui::GetMouseDragDelta(ImGuiMouseButton_Right) : ImVec2{};
		m_PanPosition.x += panningDelta.x*1.0f/m_Zoom.x;
		m_PanPosition.y += panningDelta.y*1.0f/m_Zoom.x;
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

		m_Zoom += ImGui::GetIO().MouseWheel*0.1f;
		m_Zoom = glm::clamp(m_Zoom, glm::vec3{ 0.5f }, glm::vec3{ 10.0f });

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
