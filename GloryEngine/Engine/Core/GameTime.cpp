#include "GameTime.h"
#include "Engine.h"

namespace Glory
{
    void GameTime::Initialize()
    {
        m_StartTime = Now();
        m_LastTime = TimeSinceSeconds(m_StartTime);
        m_DeltaTime = 0.0f;
    }

    void GameTime::BeginFrame()
    {
        const double time = TimeSinceSeconds(m_StartTime);
        m_DeltaTime = time - m_LastTime;
        m_LastTime = time;
    }

    void GameTime::EndFrame()
    {
        ++m_TotalFrames;
    }

    const float GameTime::GetTime() const
    {
        return m_LastTime*m_TimeScale;
    }

    const float GameTime::GetUnscaledTime() const
    {
        return m_LastTime;
    }

    const float GameTime::GetDeltaTime() const
    {
        return m_DeltaTime*m_TimeScale;
    }

    const float GameTime::GetUnscaledDeltaTime() const
    {
        return m_DeltaTime;
    }

    const float GameTime::GetTimeScale() const
    {
        return m_TimeScale;
    }

    const float GameTime::GetFrameRate() const
    {
        return 1.0f/m_DeltaTime;
    }

    const int GameTime::GetTotalFrames() const
    {
        return m_TotalFrames;
    }

    void GameTime::SetTimeScale(float scale)
    {
        m_TimeScale = scale;
    }

    uint64_t GameTime::Now()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    float GameTime::TimeSinceSeconds(uint64_t timestamp)
    {
        return (Now() - timestamp)/(1000.0f*1000.0f);
    }

    GameTime::GameTime(Engine* pEngine): m_pEngine(pEngine) {}
    GameTime::~GameTime()
    {
        m_pEngine = nullptr;
    }
}
