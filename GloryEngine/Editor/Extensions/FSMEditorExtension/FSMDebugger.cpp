#include "FSMDebugger.h"
#include "FSMEditor.h"

#include <EditorUI.h>

#include <FSMModule.h>
#include <AssetManager.h>
#include <FSM.h>
#include <WindowModule.h>

#include <StringUtils.h>

namespace Glory::Editor
{
	bool ForceInstancesFilter = false;

	FSMDebugger::FSMDebugger() : EditorWindowTemplate("Debugger", 600.0f, 600.0f), m_LastFrameInstancesCount(0)
	{
	}

	FSMDebugger::~FSMDebugger()
	{
	}

	void FSMDebugger::OnGUI()
	{
		if (!EditorApplication::GetInstance()->IsInPlayMode())
		{
			ImGui::TextUnformatted("Enter play mode to start debugging");
			m_LastFrameInstancesCount = 0;
			ForceInstancesFilter = true;
			return;
		}

		UUID& debuggingInstance = GetMainWindow()->DebuggingInstance();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		FSMModule* pFSMModule = pEngine->GetOptionalModule<FSMModule>();

		ForceInstancesFilter |= m_LastFrameInstancesCount != pFSMModule->FSMInstanceCount();
		m_LastFrameInstancesCount = pFSMModule->FSMInstanceCount();

		float start, width;
		bool openPopup = false;

		std::string currentSelectedInstanceName = "None";
		if (debuggingInstance)
		{
			const FSMState* state = pFSMModule->FSMInstance(debuggingInstance);
			if (!state) debuggingInstance = 0;
			else
			{
				const UUID fsmID = state->OriginalFSMID();
				Resource* pResource = pEngine->GetAssetManager().FindResource(fsmID);
				currentSelectedInstanceName = (pResource ? pResource->Name() + " - Instance ID: " : std::string("?Missing FSM? - Instance ID"))
					+ std::to_string(debuggingInstance);
			}
		}

		EditorUI::EmptyDropdown("Property", currentSelectedInstanceName, [&openPopup]() {
			openPopup = true;
			ForceInstancesFilter = true;
		}, start, width);

		if (openPopup)
			ImGui::OpenPopup("PropertiesDropdown");

		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		Window* pWindow = pEngine->GetMainModule<WindowModule>()->GetMainWindow();
		int mainWindowWidth, mainWindowHeight;
		pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
		ImGui::SetNextWindowPos({ windowPos.x + start, windowPos.y + cursor.y - 2.5f });
		ImGui::SetNextWindowSize({ width, mainWindowHeight - windowPos.y - cursor.y - 10.0f });
		if (ImGui::BeginPopup("PropertiesDropdown"))
		{
			static char searchBuffer[200] = "";
			static std::vector<UUID> searchResultCache;

			const bool needsFilter = EditorUI::SearchBar(ImGui::GetContentRegionAvail().x, searchBuffer, 200) || ForceInstancesFilter;
			ForceInstancesFilter = false;
			if (needsFilter)
			{
				searchResultCache.clear();
				const std::string_view search = searchBuffer;

				for (size_t i = 0; i < pFSMModule->FSMInstanceCount(); ++i)
				{
					const FSMState& state = pFSMModule->FSMInstance(i);
					const UUID fsmID = state.OriginalFSMID();
					Resource* pResource = pEngine->GetAssetManager().FindResource(fsmID);
					const std::string name = (pResource ? pResource->Name() + " - Instance ID: " : std::string("?Missing FSM? - Instance ID: "))
						+ std::to_string(debuggingInstance);
					if (!search.empty() && Utils::CaseInsensitiveSearch(search, name) != std::string::npos) continue;
					searchResultCache.push_back(state.ID());
				}
			}

			if (ImGui::Selectable("None", debuggingInstance == 0))
			{
				debuggingInstance = 0;
				GetMainWindow()->SetFSM(0);
				ImGui::CloseCurrentPopup();
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
					const UUID instanceID = *it;

					const FSMState* state = pFSMModule->FSMInstance(instanceID);
					if (!state)
					{
						ForceInstancesFilter = true;
						ImGui::PopID();
						break;
					}
					const UUID fsmID = state->OriginalFSMID();

					const std::string otherPropIDStr = std::to_string(instanceID);
					ImGui::PushID(otherPropIDStr.data());
					if (ImGui::Selectable("##select", debuggingInstance == instanceID, ImGuiSelectableFlags_AllowItemOverlap, { 0.0f, rowHeight }))
					{
						debuggingInstance = instanceID;
						GetMainWindow()->SetFSM(fsmID);
						ImGui::CloseCurrentPopup();
					}
					Resource* pResource = pEngine->GetAssetManager().FindResource(fsmID);
					const std::string name = (pResource ? pResource->Name() + " - Instance ID: " : std::string("?Missing FSM? - Instance ID: "))
						+ std::to_string(debuggingInstance);

					ImGui::SameLine();
					ImGui::TextUnformatted(name.data());
					ImGui::PopID();
				}
			}
			ImGui::EndChild();
			ImGui::EndPopup();
		}
	}

	void FSMDebugger::Update()
	{
		UUID& debuggingInstance = GetMainWindow()->DebuggingInstance();
		if (!debuggingInstance || !EditorApplication::GetInstance()->IsInPlayMode()) return;

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		FSMModule* pFSMModule = pEngine->GetOptionalModule<FSMModule>();

		const FSMState* state = pFSMModule->FSMInstance(debuggingInstance);
		if (!state) return;

		const UUID currentState = state->CurrentState();
		UUID& activeNode = GetMainWindow()->ActiveNode();
		UUID& lastActiveNode = GetMainWindow()->LastActiveNode();

		if (activeNode != currentState)
		{
			lastActiveNode = activeNode;
			activeNode = currentState;
		}
	}

	void FSMDebugger::Draw()
	{
	}

	FSMEditor* FSMDebugger::GetMainWindow()
	{
		return static_cast<FSMEditor*>(m_pOwner);
	}
}
