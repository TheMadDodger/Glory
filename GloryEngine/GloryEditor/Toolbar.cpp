#include "Toolbar.h"
#include "MainEditor.h"

namespace Glory::Editor
{
	const float Toolbar::BUTTON_SPACING = 5.0f;
	std::map<ToolChainPosition, std::vector<IToolChain*>> Toolbar::m_ToolChains;

	Toolbar::Toolbar(float toolbarSize) : TOOLBAR_SIZE(toolbarSize), m_ToolbarLength(0.0f)
	{
	}

	Toolbar::~Toolbar()
	{
		for (auto it : m_ToolChains)
		{
			it.second.clear();
		}
		m_ToolChains.clear();
	}

	void Toolbar::AddToolChain(const ToolChainPosition& toolChain, IToolChain* pToolChain)
	{
		m_ToolChains[toolChain].push_back(pToolChain);
	}

	void Toolbar::Paint()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + MainEditor::MENUBAR_SIZE));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, TOOLBAR_SIZE));
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags window_flags = 0
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoSavedSettings;

		ImGui::Begin("TOOLBAR", NULL, window_flags);

		m_ToolbarLength = ImGui::GetContentRegionAvail().x;

		LeftToolchain();
		CenterToolchain();
		RightToolchain();

		ImGui::End();
	}

	void Toolbar::LeftToolchain()
	{
		ImGui::BeginChild("LEFT_TOOLCHAIN", ImVec2(m_ToolbarLength / 3.0f, 0.0f));

		//for (size_t i = 0; i < 10; i++)
		//{
		//	std::string label = "B" + std::to_string(i);
		//	bool selected = selectedIndex == i;
		//
		//
		//	//ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5, 0.5f));
		//	//if (ImGui::Selectable(label.c_str(), selected, 0, buttonSize))
		//	//{
		//	//	selectedIndex = i;
		//	//}
		//	//ImGui::PopStyleVar();
		//
		//	if (selectedIndex == i)
		//	{
		//		ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		//
		//		ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
		//		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeColor);
		//		ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
		//		ImGui::Button(label.c_str(), buttonSize);
		//		ImGui::PopStyleColor(3);
		//	}
		//	else if (ImGui::Button(label.c_str(), buttonSize))
		//	{
		//		selectedIndex = i;
		//	}
		//	if (i < 9) ImGui::SameLine(0.0f, 0.0f);
		//}

		float start = ImGui::GetStyle().WindowPadding.x;
		DrawSubBar(start, ToolChainPosition::Left);
		ImGui::EndChild();
	}

	void Toolbar::CenterToolchain()
	{
		ImGui::SameLine();
		ImGui::BeginChild("CENTER_TOOLCHAIN", ImVec2(m_ToolbarLength / 3.0f, 0.0f));
		float barWidth = ImGui::GetContentRegionAvail().x;
		float startPos = barWidth / 2.0f - m_LastFrameToolBarLengths[ToolChainPosition::Center] / 2.0;
		DrawSubBar(startPos, ToolChainPosition::Center);
		ImGui::EndChild();
	}

	void Toolbar::RightToolchain()
	{
		ImGui::SameLine();
		ImGui::BeginChild("RIGHT_TOOLCHAIN", ImVec2(m_ToolbarLength / 3.0f - 5.0f, 0.0f));
		float barWidth = ImGui::GetContentRegionAvail().x;
		float startPos = barWidth - m_LastFrameToolBarLengths[ToolChainPosition::Right];
		DrawSubBar(startPos, ToolChainPosition::Right);
		ImGui::EndChild();
	}

	void Toolbar::DrawSubBar(float startPos, const ToolChainPosition& toolChain)
	{
		float toolBarHeight = ImGui::GetContentRegionAvail().y;
		ImVec2 buttonSize(toolBarHeight, toolBarHeight);

		float cursor = startPos;
		for (size_t i = 0; i < m_ToolChains[toolChain].size(); i++)
		{
			IToolChain* pToolChain = m_ToolChains[toolChain][i];
			pToolChain->DrawToolchain(cursor, buttonSize);
			cursor += 5.0f;
		}

		m_LastFrameToolBarLengths[toolChain] = cursor - startPos;
	}
}
