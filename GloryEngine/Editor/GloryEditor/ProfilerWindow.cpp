#include "ProfilerWindow.h"
#include <ProfilerModule.h>
#include <Game.h>
#include <Engine.h>
#include <imgui.h>
#include <implot.h>
#include <GameTime.h>
#include <implot_internal.h>

namespace Glory::Editor
{
	bool ProfilerWindow::m_IsRecording = true;
	const float ProfilerWindow::SAMPLE_INTERVALS = 0.1f;
	std::unordered_map<std::string, std::chrono::time_point<std::chrono::steady_clock>> ProfilerWindow::m_LastSamples;
	std::unordered_map<std::string, std::vector<ProfilerThreadSample>> ProfilerWindow::m_Samples;
	std::unordered_map<std::string, std::vector<float>> ProfilerWindow::m_TimeSamples;
	std::unordered_map<std::string, std::vector<float>> ProfilerWindow::m_TimeFrames;
	std::unordered_map<std::string, int> ProfilerWindow::m_WriteIndices;
	std::mutex ProfilerWindow::m_RecordMutex;

	ProfilerWindow::ProfilerWindow() : EditorWindowTemplate("Profiler", 1280, 720), m_CurrentlyInspectingSampleIndex(-1) {}
	ProfilerWindow::~ProfilerWindow() {}

	void ProfilerWindow::OnGUI()
	{
		MenuBar();
		GraphGUI();
		InspectorGUI();
	}

	void ProfilerWindow::MenuBar()
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		if (ImGui::BeginChild("Menu Bar", ImVec2(windowSize.x, 24.0f)))
		{

			if (ImGui::Button("Record"))
			{
				m_IsRecording = !m_IsRecording;
				m_CurrentlyInspectingSampleIndex = -1;
			}
		}

		ImGui::EndChild();
	}

	void ProfilerWindow::GraphGUI()
	{
		ImVec2 plotSize = ImGui::GetContentRegionAvail();
		plotSize.y *= 0.4f;

		float minPlotHeight = 400.0f;
		if (plotSize.y <= minPlotHeight) plotSize.y = minPlotHeight;

		if (ImGui::BeginChild("Graph", plotSize))
		{
			std::unique_lock<std::mutex> lock(m_RecordMutex);

			std::vector<std::pair<std::string, float>> maxValues;

			// Find limits
			float frameMin = 0, frameMax = 1;
			float timeMax = 0.0f;
			for (auto it = m_TimeSamples.begin(); it != m_TimeSamples.end(); it++)
			{
				const std::string& name = it->first;
				std::vector<float> samples = it->second;
				std::vector<float> frames = m_TimeFrames[name];

				float newFrameMin = frames[0];
				float newFrameMax = frames[frames.size() - 1];

				if (frameMin < newFrameMin) frameMin = newFrameMin;
				if (frameMax < newFrameMax) frameMax = frames[frames.size() - 1];

				std::sort(samples.begin(), samples.end());
				float newTimeMax = samples[samples.size() - 1];
				maxValues.push_back({ name, newTimeMax });
				if (timeMax < newTimeMax) timeMax = newTimeMax;
			}

			std::sort(maxValues.begin(), maxValues.end(), [](std::pair<std::string, float>& a, std::pair<std::string, float>& b)
			{
				return a.second > b.second;
			});

			if (timeMax == 0.0f) timeMax = 10.0f;

			if (ImPlot::BeginPlot("Threads", plotSize))
			{
				ImPlot::SetupAxes("Frame", "Time");
				ImPlot::SetupAxesLimits(frameMin, frameMax, 0.0, timeMax, ImPlotCond_Always);

				ImDrawList* draw_list = ImPlot::GetPlotDrawList();
				double half_width = 0.25f;

				if (m_CurrentlyInspectingSampleIndex != -1)
				{
					int gridX = m_CurrentlyInspectingSampleIndex + (int)frameMin;
					float tool_l = ImPlot::PlotToPixels(gridX - half_width * 1.5, 0).x;
					float tool_r = ImPlot::PlotToPixels(gridX + half_width * 1.5, 0).x;
					float tool_t = ImPlot::GetPlotPos().y;
					float tool_b = tool_t + ImPlot::GetPlotSize().y;
					ImPlot::PushPlotClipRect();
					draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
					ImPlot::PopPlotClipRect();
				}

				if (ImPlot::IsPlotHovered()) {
					ImPlotPoint mouse = ImPlot::GetPlotMousePos();

					int gridX = (int)mouse.x;

					float tool_l = ImPlot::PlotToPixels(gridX - half_width * 1.5, mouse.y).x;
					float tool_r = ImPlot::PlotToPixels(gridX + half_width * 1.5, mouse.y).x;
					float tool_t = ImPlot::GetPlotPos().y;
					float tool_b = tool_t + ImPlot::GetPlotSize().y;
					ImPlot::PushPlotClipRect();
					draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
					ImPlot::PopPlotClipRect();

					int index = gridX - (int)frameMin;
					ImGui::BeginTooltip();
					for (size_t i = 0; i < maxValues.size(); i++)
					{
						const std::string& name = maxValues[i].first;
						std::vector<float> samples = m_TimeSamples[name];
						std::vector<float> frames = m_TimeFrames[name];

						int frame = (int)frames[index];
						float sample = samples[index];

						if (i == 0) ImGui::Text("Frame: %d", frame);
						ImGui::Text("%s: %.3f", name.c_str(), sample);
					}
					ImGui::EndTooltip();

					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						m_IsRecording = false;
						m_CurrentlyInspectingSampleIndex = index;
					}
				}

				ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);

				for (size_t i = 0; i < maxValues.size(); i++)
				{
					const std::string& name = maxValues[i].first;
					std::vector<float> samples = m_TimeSamples[name];
					std::vector<float> frames = m_TimeFrames[name];
					if (frames.size() <= 1) continue;
					ImPlot::PlotShaded<float>(name.c_str(), (const float*)frames.data(), (const float*)samples.data(), (int)samples.size());
					ImPlot::PlotLine<float>(name.c_str(), (const float*)frames.data(), (const float*)samples.data(), (int)samples.size());
				}

				ImPlot::PopStyleVar();
				ImPlot::EndPlot();
			}

			lock.unlock();

		}
		ImGui::EndChild();
	}

	void ProfilerWindow::InspectorGUI()
	{
		if (ImGui::BeginChild("Inspector"))
		{
			if (m_IsRecording)
			{
				ImGui::Text("You cannot inspect a sample while the profiler is recording.");
				ImGui::EndChild();
				return;
			}
			if (m_CurrentlyInspectingSampleIndex == -1)
			{
				ImGui::Text("Select a sample to inspect.");
				ImGui::EndChild();
				return;
			}

			if (m_Samples.size() <= 0)
			{
				ImGui::Text("No samples available.");
				ImGui::EndChild();
				return;
			}

			InspectorMenu();

			switch (m_InspectorMode)
			{
			case Glory::Editor::Timeline:
				InspectorTimelineGUI();
				break;
			case Glory::Editor::Hierarchy:
				InspectorTableGUI();
				break;
			default:
				break;
			}
		}
		ImGui::EndChild();
	}

	void ProfilerWindow::InspectorMenu()
	{
		static std::string inspectorModeName = "Timeline";

		if (ImGui::BeginCombo("##Inspector Mode", inspectorModeName.c_str()))
		{
			if (ImGui::Selectable("Timeline", m_InspectorMode == InspectorMode::Timeline))
			{
				m_InspectorMode = InspectorMode::Timeline;
				inspectorModeName = "Timeline";
			}
			if (ImGui::Selectable("Hierarchy", m_InspectorMode == InspectorMode::Hierarchy))
			{
				m_InspectorMode = InspectorMode::Hierarchy;
				inspectorModeName = "Hierarchy";
			}
			ImGui::EndCombo();
		}
	}

	void ProfilerWindow::InspectorTableGUI()
	{
		const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
		ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

		if (ImGui::BeginTable("Hierarchy", 2, flags))
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
			ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
			ImGui::TableHeadersRow();

			for (auto it = m_Samples.begin(); it != m_Samples.end(); it++)
			{
				if (it->second.size() <= m_CurrentlyInspectingSampleIndex) continue;
				ThreadSampleTableGUI(it->second[m_CurrentlyInspectingSampleIndex]);
			}

			ImGui::EndTable();
		}
	}

	void ProfilerWindow::InspectorTimelineGUI()
	{
		static ImGuiTableFlags flags =
			//ImGuiTableFlags_Resizable
			ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
			| ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
			| ImGuiTableFlags_SizingFixedFit;

		static float start = 0;
		static float end = 100;

		if (ImGui::BeginTable("Timeline", 2, flags, ImVec2(0, 0), 0.0f))
		{
			ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 0);
			ImGui::TableSetupColumn("Timeline", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.0f, 1);
			//ImGui::TableSetupColumn("Project Path", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 1);

			ImGui::TableHeadersRow();

			for (size_t row_n = 0; row_n < 10; row_n++)
			{
				ImGui::PushID((int)row_n);
				ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);

				ImGui::TableSetColumnIndex(0);
				ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

				if (ImGui::Selectable("##", false, selectable_flags, ImVec2(0, 0.0f)))
				{
					
				}

				ImGui::Text("Test %d", row_n);

				ImGui::TableSetColumnIndex(1);
				if (ImPlot::BeginPlot("Test Plot"))
				{

					ImPlot::EndPlot();
				}

				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}

	void ProfilerWindow::ThreadSampleTableGUI(ProfilerThreadSample sample)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		const bool is_folder = (sample.SampleCount() > 0);
		if (is_folder)
		{
			bool open = ImGui::TreeNodeEx(sample.Name().c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
			ImGui::TableNextColumn();
			ImGui::Text("%.3f ms", sample.GetDuration<float, std::milli>());
			if (open)
			{
				for (size_t i = 0; i < sample.SampleCount(); i++)
				{
					ProfilerSample* pSubSample = sample.GetSample(i);
					SampleTableGUI(pSubSample);
				}
				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::TreeNodeEx(sample.Name().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
			ImGui::TableNextColumn();
			ImGui::Text("%.3f ms", sample.GetDuration<float, std::milli>());
		}
	}

	void ProfilerWindow::SampleTableGUI(ProfilerSample* pSample)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		const bool is_folder = (pSample->SubSampleCount() > 0);
		if (is_folder)
		{
			bool open = ImGui::TreeNodeEx(pSample->Name().c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
			ImGui::TableNextColumn();
			ImGui::Text("%.3f ms", pSample->GetDuration<float, std::milli>());
			if (open)
			{
				for (size_t i = 0; i < pSample->SubSampleCount(); i++)
				{
					ProfilerSample* pSubSample = pSample->GetSubSample(i);
					SampleTableGUI(pSubSample);
				}
				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::TreeNodeEx(pSample->Name().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
			ImGui::TableNextColumn();
			ImGui::Text("%.3f ms", pSample->GetDuration<float, std::milli>());
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

		std::string name = sample.Name();
		std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float, std::ratio<1, 1>> duration = currentTime - m_LastSamples[name];
		if (duration.count() < SAMPLE_INTERVALS) return;
		m_LastSamples[name] = std::chrono::steady_clock::now();

		std::unique_lock<std::mutex> lock(m_RecordMutex);
		m_Samples[name].push_back(sample);
		if (m_Samples[name].size() >= MAX_SAMPLE_RECORDS)
		{
			m_Samples[name].erase(m_Samples[name].begin());
		}

		float time = sample.GetDuration<float, std::milli>();

		if (m_TimeSamples.find(name) == m_TimeSamples.end())
		{
			m_TimeSamples[name].resize(MAX_SAMPLE_RECORDS, 0.0f);
			m_WriteIndices[name] = 0;
			m_TimeFrames[name].resize(MAX_SAMPLE_RECORDS);
			for (size_t i = 0; i < m_TimeFrames[name].size(); i++)
			{
				m_TimeFrames[name][i] = (float)i;
			}
		}

		if (m_WriteIndices[name] < MAX_SAMPLE_RECORDS)
		{
			m_TimeSamples[name][m_WriteIndices[name]] = time;
			++m_WriteIndices[name];
			return;
		}

		float frame = m_TimeFrames[name][m_TimeFrames[name].size() - 1];

		m_TimeSamples[name].push_back(time);
		m_TimeFrames[name].push_back(frame + 1);
		if (m_TimeSamples[name].size() >= MAX_SAMPLE_RECORDS)
		{
			m_TimeSamples[name].erase(m_TimeSamples[name].begin());
		}
		if (m_TimeFrames[name].size() >= MAX_SAMPLE_RECORDS)
		{
			m_TimeFrames[name].erase(m_TimeFrames[name].begin());
		}
		lock.unlock();
	}
}
