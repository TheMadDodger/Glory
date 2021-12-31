#pragma once
#include "EditorWindow.h"
#include <ProfilerThreadSample.h>

namespace Glory::Editor
{
    class ProfilerWindow : public EditorWindowTemplate<ProfilerWindow>
    {
    public:
        ProfilerWindow();
        virtual ~ProfilerWindow();
        virtual void OnOpen() override;
        virtual void OnClose() override;

    private:
        virtual void OnGUI() override;
        void SampleGUI(ProfilerSample* pSample);

        static void StoreSampleRecord(const ProfilerThreadSample& sample);

    private:
        int m_CurrentlyInspectingSampleIndex;

        static bool m_IsRecording;
        static const size_t MAX_SAMPLE_RECORDS = 100;
        static size_t m_CurrentSampleRecordSize;
        static int m_CurrentSampleWrite;
        static ProfilerThreadSample m_SampleRecords[MAX_SAMPLE_RECORDS];
    };
}
