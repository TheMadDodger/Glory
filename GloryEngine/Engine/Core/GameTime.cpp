#include "GameTime.h"
#include "Engine.h"
#include "TimerModule.h"

namespace Glory
{
    const float GameTime::GetTime()
    {
        return GetTimer()->CalculateTime(false);
    }

    const float GameTime::GetUnscaledTime()
    {
        return GetTimer()->CalculateTime(true);
    }

    const float GameTime::GetTimeScale()
    {
        return GetTimer()->m_TimeScale;
    }

    const float GameTime::GetFrameRate()
    {
        float deltaTime = GetUnscaledDeltaTime<float, std::ratio<1, 1>>();
        return 1.0f / deltaTime;
    }

    const int GameTime::GetTotalFrames()
    {
        return GetTimer()->m_TotalGraphicsFrames;
    }

    const int GameTime::GetTotalGameFrames()
    {
        return GetTimer()->m_TotalGameFrames;
    }

    void GameTime::SetTimeScale(float scale)
    {
        GetTimer()->m_TimeScale = scale;
    }

    TimerModule* GameTime::GetTimer()
    {
        return m_pEngine->GetInternalModule<TimerModule>();
    }

    GameTime::GameTime(Engine* pEngine): m_pEngine(pEngine) {}
    GameTime::~GameTime()
    {
        m_pEngine = nullptr;
    }
}
