#include "EditorPlayer.h"
#include "EditorApplication.h"
#include "EditorSceneManager.h"
#include "Selection.h"
#include "Undo.h"
#include "SelectAction.h"
#include "DeselectAction.h"
#include "EditorAssets.h"
#include "EditorApplication.h"
#include "ImGuiHelpers.h"
#include "ProjectSettings.h"

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	std::vector<IPlayModeHandler*> EditorPlayer::m_pSceneLoopHandlers;

	void EditorPlayer::RegisterLoopHandler(IPlayModeHandler* pEditorLoopHandler)
	{
		m_pSceneLoopHandlers.push_back(pEditorLoopHandler);
	}

	void EditorPlayer::Start()
	{
		ProjectSettings::OnStartPlay();

		Object* pSelected = Selection::GetActiveObject();
		if (pSelected != nullptr) m_SelectedObjectBeforeStart = pSelected->GetUUID();
		Selection::Clear();
		m_UndoHistoryIndex = Undo::GetHistorySize();
		YAML::Emitter out;
		EditorSceneManager::SerializeOpenScenes(out);
		m_SerializedScenes = out.c_str();

		//if (pSelected) Selection::SetActiveObject(pSelected);

		Engine* pEngine = Game::GetGame().GetEngine();
		for (size_t i = 0; i < m_pSceneLoopHandlers.size(); i++)
		{
			IPlayModeHandler* pPlayModeHandler = m_pSceneLoopHandlers[i];
			Module* pModule = pEngine->GetModule(pPlayModeHandler->ModuleName());
			if (!pModule) continue;
			pPlayModeHandler->HandleStart(pModule);
		}
	}

	void EditorPlayer::Stop()
	{
		ProjectSettings::OnStopPlay();

		Engine* pEngine = Game::GetGame().GetEngine();
		for (size_t i = 0; i < m_pSceneLoopHandlers.size(); i++)
		{
			IPlayModeHandler* pPlayModeHandler = m_pSceneLoopHandlers[i];
			Module* pModule = pEngine->GetModule(pPlayModeHandler->ModuleName());
			if (!pModule) continue;
			pPlayModeHandler->HandleStop(pModule);
		}

		m_IsPaused = false;
		m_FrameRequested = false;

		//Object* pSelected = Selection::GetActiveObject();
		//UUID toSelect = 0;
		//if (pSelected != nullptr) toSelect = pSelected->GetUUID();

		Selection::Clear();
		EditorSceneManager::CloseAll();
		YAML::Node node = YAML::Load(m_SerializedScenes);
		EditorSceneManager::OpenAllFromNode(node);

		//pSelected = Object::FindObject(toSelect);
		//Selection::SetActiveObject(pSelected);
		Undo::ClearHistoryFrom(m_UndoHistoryIndex);

		//toSelect = 0;
		//if (pSelected != nullptr) toSelect = pSelected->GetUUID();
		//if (m_SelectedObjectBeforeStart && m_SelectedObjectBeforeStart != toSelect)
		//{
		//	Undo::StartRecord("Selection");
		//	Undo::AddAction(new DeselectAction(m_SelectedObjectBeforeStart));
		//	if (toSelect) Undo::AddAction(new SelectAction(toSelect));
		//	Undo::StopRecord();
		//}
		//m_SelectedObjectBeforeStart = 0;
	}

	void EditorPlayer::TogglePauze()
	{
		m_IsPaused = !m_IsPaused;
		m_FrameRequested = false;
	}

	void EditorPlayer::TickFrame()
	{
		if (!m_IsPaused) return;
		m_FrameRequested = true;
	}

	bool EditorPlayer::HandleModuleLoop(Module* pModule)
	{
		const ModuleMetaData& metaData = pModule->GetMetaData();
		if (metaData.Type() != ModuleType::MT_SceneManagement) return false;

		if (EditorApplication::CurrentMode() == EditorMode::M_Play)
		{
			if (!m_IsPaused || m_FrameRequested) pModule->GetEngine()->CallModuleUpdate(pModule);
			m_FrameRequested = false;
		}
		else
		{
			auto it = std::find_if(m_pSceneLoopHandlers.begin(), m_pSceneLoopHandlers.end(), [&](IPlayModeHandler* pEditorLoopHandler)
			{
				const std::string& moduleName = pEditorLoopHandler->ModuleName();
				return moduleName == metaData.Name();
			});
			if (it == m_pSceneLoopHandlers.end()) return false;
			IPlayModeHandler* pEditorLoopHandler = *it;
			pEditorLoopHandler->HandleUpdate(pModule);
		}

		pModule->GetEngine()->CallModuleDraw(pModule);
		return true;
	}

	void EditorPlayer::DrawToolchain(float& cursor, const ImVec2& maxButtonSize)
	{
		ImGui::SameLine(cursor);
		ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		ImVec4 hoverColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);

		int styleColorCount = 0;

		switch (EditorApplication::CurrentMode())
		{
		case EditorMode::M_Edit:

			break;
		case EditorMode::M_Play:
			ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
			styleColorCount = 3;
			break;
		case EditorMode::M_EnteringPlay:
			ImGui::PushStyleColor(ImGuiCol_Button, hoverColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, hoverColor);
			styleColorCount = 3;
			break;
		case EditorMode::M_ExitingPlay:

			break;
		default:
			break;
		}

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();
		if (ImGui::Button(ICON_FA_PLAY, maxButtonSize))
		{
			EditorApplication::TogglePlay();
		}
		ImGui::PopStyleColor(styleColorCount);

		cursor += maxButtonSize.x;
		ImGui::SameLine(cursor);

		styleColorCount = 0;
		if (m_IsPaused)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
			styleColorCount = 3;
		}

		if (ImGui::Button(ICON_FA_PAUSE, maxButtonSize))
		{
			EditorApplication::TogglePause();
		}
		ImGui::PopStyleColor(styleColorCount);

		cursor += maxButtonSize.x;
		ImGui::SameLine(cursor);
		if (ImGui::Button(ICON_FA_FORWARD_STEP, maxButtonSize))
		{
			EditorApplication::TickFrame();
		}
	}

	EditorPlayer::EditorPlayer() : m_SerializedScenes(""), m_UndoHistoryIndex(0), m_IsPaused(false), m_FrameRequested(false)
	{
		Toolbar::AddToolChain(ToolChainPosition::Center, this);
	}

	EditorPlayer::~EditorPlayer()
	{
	}
}
