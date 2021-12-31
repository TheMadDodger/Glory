#include "ProfilerWindow.h"
#include <ProfilerModule.h>
#include <Game.h>
#include <imgui.h>

namespace Glory::Editor
{
	bool ProfilerWindow::m_IsRecording = true;
	size_t ProfilerWindow::m_CurrentSampleRecordSize = 0;
	int ProfilerWindow::m_CurrentSampleWrite = -1;
	ProfilerThreadSample ProfilerWindow::m_SampleRecords[MAX_SAMPLE_RECORDS];

	ProfilerWindow::ProfilerWindow() : EditorWindowTemplate("Profiler", 1280, 720), m_CurrentlyInspectingSampleIndex(-1) {}
	ProfilerWindow::~ProfilerWindow() {}

	void ProfilerWindow::OnGUI()
	{
		if (ImGui::Button("Record"))
		{
			m_IsRecording = !m_IsRecording;
			m_CurrentlyInspectingSampleIndex = -1;
		}

		if (!m_IsRecording)
		{
			ImGui::InputInt("Inspect Sample Index", &m_CurrentlyInspectingSampleIndex);
		}

		if (m_CurrentlyInspectingSampleIndex == -1) return;

		ProfilerThreadSample sample = m_SampleRecords[m_CurrentlyInspectingSampleIndex];

		bool hasChildren = sample.SampleCount() > 0;
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Selected;

		if (!hasChildren)
			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (ImGui::TreeNodeEx(sample.Name().c_str(), node_flags))
		{
			for (size_t i = 0; i < sample.SampleCount(); i++)
			{
				ProfilerSample* pSample = sample.GetSample(i);
				SampleGUI(pSample);
			}
			if (hasChildren) ImGui::TreePop();
		}
	}

	void ProfilerWindow::SampleGUI(ProfilerSample* pSample)
	{
		bool hasChildren = pSample->SubSampleCount() > 0;
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Selected;

		if (!hasChildren)
			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (ImGui::TreeNodeEx(pSample->Name().c_str(), node_flags))
		{
			float duration = pSample->GetDuration<float, std::milli>();

			ImGui::Text("Time: %.3f ms", duration);

			for (size_t i = 0; i < pSample->SubSampleCount(); i++)
			{
				ProfilerSample* pSubSample = pSample->GetSubSample(i);
				SampleGUI(pSubSample);
			}
			if (hasChildren) ImGui::TreePop();
		}
	}

	void ProfilerWindow::OnOpen()
	{
		ProfilerModule* pProfiler = Game::GetGame().GetEngine()->GetProfilerModule();
		pProfiler->RegisterRecordCallback(ProfilerWindow::StoreSampleRecord);
		pProfiler->EnableSampleCollecting(true);
	}

	void ProfilerWindow::OnClose()
	{
		ProfilerModule* pProfiler = Game::GetGame().GetEngine()->GetProfilerModule();
		pProfiler->RegisterRecordCallback(NULL);
		pProfiler->EnableSampleCollecting(false);
	}

	void ProfilerWindow::StoreSampleRecord(const ProfilerThreadSample& sample)
	{
		if (!m_IsRecording) return;
		//std::unique_lock<std::mutex> lock(m_RecordMutex);

		++m_CurrentSampleWrite;
		if (m_CurrentSampleWrite >= MAX_SAMPLE_RECORDS) m_CurrentSampleWrite = 0;
		if (m_CurrentSampleRecordSize < MAX_SAMPLE_RECORDS) ++m_CurrentSampleRecordSize;
		m_SampleRecords[m_CurrentSampleWrite] = sample;

		//lock.unlock();
	}
}
