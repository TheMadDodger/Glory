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
			return GetUnscaledDeltaTime<T, Ratio>()*GetTimer()->m_TimeScale;
		}

		template<typename T, typename Ratio>
		const float GetUnscaledDeltaTime()
		{
			std::chrono::duration<T, Ratio> deltaTime = GetTimer()->m_LastFrameEnd - GetTimer()->m_LastFrameStart;
			return deltaTime.count();
		}

		const float GetTime();
		const float GetUnscaledTime();
		const float GetTimeScale();
		const float GetFrameRate();
		const int GetTotalFrames();
		void SetTimeScale(float scale);

	private:
		friend class Engine;
		TimerModule* GetTimer();

		GameTime(Engine* pEngine);

		Engine* m_pEngine;
	};
}
