#pragma once
#include "EditorWindow.h"
#include <chrono>

namespace Glory::Editor
{
    class PerformanceMetrics : public EditorWindowTemplate<PerformanceMetrics>
    {
    public:
        PerformanceMetrics();
        virtual ~PerformanceMetrics();

    private:
        virtual void OnGUI() override;

        void Refresh();

    private:
        static const float REFRESHINTERVALS;
        std::chrono::time_point<std::chrono::system_clock> m_LastRefresh;

        float m_LastFrameTime;
        float m_LastFramerate;
    };
}
