#pragma once
#include "TimerModule.h"

//#define TIMER Game::GetGame().GetEngine()->GetTimerModule()

namespace Glory
{
	class Time
	{
	public:
		template<typename T, typename Ratio>
		static const float GetDeltaTime()
		{
			T gameDelta = GetGameDeltaTime<T, Ratio>();
			T graphicsDelta = GetGraphicsDeltaTime<T, Ratio>();
			return gameDelta + graphicsDelta;
		}

		template<typename T, typename Ratio>
		static const float GetGameDeltaTime()
		{
			return GetUnscaledGameDeltaTime<T, Ratio>() * GetTimer()->m_TimeScale;
		}

		template<typename T, typename Ratio>
		static const float GetGraphicsDeltaTime()
		{
			return GetUnscaledGraphicsDeltaTime<T, Ratio>() * GetTimer()->m_TimeScale;
		}

		template<typename T, typename Ratio>
		static const float GetUnscaledDeltaTime()
		{
			T gameDelta = GetUnscaledGameDeltaTime<T, Ratio>();
			T graphicsDelta = GetUnscaledGraphicsDeltaTime<T, Ratio>();
			return gameDelta + graphicsDelta;
		}

		template<typename T, typename Ratio>
		static const float GetUnscaledGameDeltaTime()
		{
			std::chrono::duration<T, Ratio> deltaTime = GetTimer()->m_LastGameThreadFrameEnd - GetTimer()->m_LastGameThreadFrameStart;
			return deltaTime.count();
		}

		template<typename T, typename Ratio>
		static const float GetUnscaledGraphicsDeltaTime()
		{
			std::chrono::duration<T, Ratio> deltaTime = GetTimer()->m_LastGraphicsThreadFrameEnd - GetTimer()->m_LastGraphicsThreadFrameStart;
			return deltaTime.count();
		}

		static const float GetTime();
		static const float GetUnscaledTime();
		static const float GetTimeScale();
		static const float GetFrameRate();
		static const int GetTotalFrames();
		static const int GetTotalGameFrames();
		static void SetTimeScale(float scale);

	private:
		static TimerModule* GetTimer();

		Time();
		virtual ~Time();
	};
}
