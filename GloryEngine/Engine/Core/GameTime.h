#pragma once
#include "TimerModule.h"

namespace Glory
{
	class Engine;

	class GameTime
	{
	public:
		virtual ~GameTime();

		template<typename T, typename Ratio>
		const float GetDeltaTime()
		{
			T gameDelta = GetGameDeltaTime<T, Ratio>();
			T graphicsDelta = GetGraphicsDeltaTime<T, Ratio>();
			return gameDelta + graphicsDelta;
		}

		template<typename T, typename Ratio>
		const float GetGameDeltaTime()
		{
			return GetUnscaledGameDeltaTime<T, Ratio>() * GetTimer()->m_TimeScale;
		}

		template<typename T, typename Ratio>
		const float GetGraphicsDeltaTime()
		{
			return GetUnscaledGraphicsDeltaTime<T, Ratio>() * GetTimer()->m_TimeScale;
		}

		template<typename T, typename Ratio>
		const float GetUnscaledDeltaTime()
		{
			T gameDelta = GetUnscaledGameDeltaTime<T, Ratio>();
			T graphicsDelta = GetUnscaledGraphicsDeltaTime<T, Ratio>();
			return gameDelta + graphicsDelta;
		}

		template<typename T, typename Ratio>
		const float GetUnscaledGameDeltaTime()
		{
			std::chrono::duration<T, Ratio> deltaTime = GetTimer()->m_LastGameThreadFrameEnd - GetTimer()->m_LastGameThreadFrameStart;
			return deltaTime.count();
		}

		template<typename T, typename Ratio>
		const float GetUnscaledGraphicsDeltaTime()
		{
			std::chrono::duration<T, Ratio> deltaTime = GetTimer()->m_LastGraphicsThreadFrameEnd - GetTimer()->m_LastGraphicsThreadFrameStart;
			return deltaTime.count();
		}

		const float GetTime();
		const float GetUnscaledTime();
		const float GetTimeScale();
		const float GetFrameRate();
		const int GetTotalFrames();
		const int GetTotalGameFrames();
		void SetTimeScale(float scale);

	private:
		friend class Engine;
		TimerModule* GetTimer();

		GameTime(Engine* pEngine);

		Engine* m_pEngine;
	};
}
