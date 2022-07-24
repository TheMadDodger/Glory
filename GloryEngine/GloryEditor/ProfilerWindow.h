#pragma once
#include "EditorWindow.h"
#include <ProfilerThreadSample.h>
#include <unordered_map>
#include <list>
#include <mutex>
#include "ProfilerTimeline.h"

namespace Glory::Editor
{
    enum InspectorMode : int
    {
        Timeline,
        Hierarchy,
    };

    class ProfilerWindow : public EditorWindowTemplate<ProfilerWindow>
    {
    public:
        ProfilerWindow();
        virtual ~ProfilerWindow();
        virtual void OnOpen() override;
        virtual void OnClose() override;

    private:
        virtual void OnGUI() override;
        static void StoreSampleRecord(const ProfilerThreadSample& sample);

    private:
        void MenuBar();
        void GraphGUI();
        void InspectorGUI();
        void InspectorMenu();
        void InspectorTableGUI();
        void ThreadSampleTableGUI(ProfilerThreadSample sample);
        void SampleTableGUI(ProfilerSample* pSample);

        void InspectorTimelineGUI();

    private:
        int m_CurrentlyInspectingSampleIndex;
        InspectorMode m_InspectorMode = InspectorMode::Timeline;

        static bool m_IsRecording;
        static const size_t MAX_SAMPLE_RECORDS = 100;
        static const float SAMPLE_INTERVALS;
        static std::unordered_map<std::string, std::chrono::time_point<std::chrono::steady_clock>> m_LastSamples;
        static std::unordered_map<std::string, std::vector<ProfilerThreadSample>> m_Samples;
        static std::unordered_map<std::string, std::vector<float>> m_TimeSamples;
        static std::unordered_map<std::string, std::vector<float>> m_TimeFrames;
        static std::unordered_map<std::string, int> m_WriteIndices;

        static std::mutex m_RecordMutex;

        ProfilerTimeline mySequence;
    };
}
