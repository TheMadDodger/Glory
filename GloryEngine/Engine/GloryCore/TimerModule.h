#pragma once
#include "Module.h"
#include <chrono>
#include <ratio>

namespace Glory
{
    //using ns = std::chrono::duration<float, std::sec>;
    using ns = std::chrono::duration<float, std::nano>;
    using seconds = std::chrono::duration<float, std::ratio<1, 1>>;

    class TimerModule : public Module
    {
    public:
        TimerModule();
        virtual ~TimerModule();

        virtual const std::type_info& GetModuleType() override;

    private:
        virtual void Initialize() override;
        virtual void Cleanup() override;
        void OnGameThreadFrameStart() override;
        void OnGameThreadFrameEnd() override;
        void OnGraphicsThreadFrameStart() override;
        void OnGraphicsThreadFrameEnd() override;

    private:
        float CalculateTime(bool unscaled);

    private:
        friend class Time;

        float m_TimeScale = 1.0f;
        int m_TotalGameFrames = 0;
        int m_TotalGraphicsFrames = 0;

    private:
        std::chrono::time_point<std::chrono::steady_clock> m_CurrentGameThreadFrameStart;
        std::chrono::time_point<std::chrono::steady_clock> m_LastGameThreadFrameStart;
        std::chrono::time_point<std::chrono::steady_clock> m_LastGameThreadFrameEnd;

        std::chrono::time_point<std::chrono::steady_clock> m_CurrentGraphicsThreadFrameStart;
        std::chrono::time_point<std::chrono::steady_clock> m_LastGraphicsThreadFrameStart;
        std::chrono::time_point<std::chrono::steady_clock> m_LastGraphicsThreadFrameEnd;

        std::chrono::time_point<std::chrono::steady_clock> m_AppStart;
    };
}
