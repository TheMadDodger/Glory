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
        void OnBeginFrame() override;
        void OnEndFrame() override;

    private:
        float CalculateTime(bool unscaled);

    private:
        friend class GameTime;

        float m_TimeScale = 1.0f;
        int m_TotalFrames = 0;

    private:
        std::chrono::time_point<std::chrono::steady_clock> m_CurrentFrameStart;
        std::chrono::time_point<std::chrono::steady_clock> m_LastFrameStart;
        std::chrono::time_point<std::chrono::steady_clock> m_LastFrameEnd;
        std::chrono::time_point<std::chrono::steady_clock> m_AppStart;
    };
}
