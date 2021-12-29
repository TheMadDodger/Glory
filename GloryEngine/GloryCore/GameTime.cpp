#include "GameTime.h"
#include "Game.h"
#include "Engine.h"
#include "TimerModule.h"

#define TIMER Game::GetGame().GetEngine()->GetTimerModule()

namespace Glory
{
    const float Time::GetTime()
    {
        return TIMER->CalculateTime(false);
    }

    const float Time::GetUnscaledTime()
    {
        return TIMER->CalculateTime(true);
    }

    const float Time::GetTimeScale()
    {
        return TIMER->m_TimeScale;
    }

    const float Time::GetFrameRate()
    {
        float deltaTime = GetUnscaledDeltaTime<float, std::ratio<1, 1>>();
        return 1.0f / deltaTime;
    }

    const int Time::GetTotalFrames()
    {
        return TIMER->m_TotalGraphicsFrames;
    }

    const int Time::GetTotalGameFrames()
    {
        return TIMER->m_TotalGameFrames;
    }

    void Time::SetTimeScale(float scale)
    {
        TIMER->m_TimeScale = scale;
    }

    TimerModule* Time::GetTimer()
    {
        return TIMER;
    }

    Time::Time() {}
    Time::~Time() {}
}
