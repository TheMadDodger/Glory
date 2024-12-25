#pragma once
#include <cstdint>

namespace Glory
{
	class Engine;

	class GameTime
	{
	public:
		virtual ~GameTime();

		void Initialize();

		void BeginFrame();
		void EndFrame();

		const float GetTime() const;
		const float GetUnscaledTime() const;
		const float GetDeltaTime() const;
		const float GetUnscaledDeltaTime() const;
		const float GetTimeScale() const;
		const float GetFrameRate() const;
		const int GetTotalFrames() const;
		void SetTimeScale(float scale);

		static uint64_t Now();
		static float TimeSinceSeconds(uint64_t timestamp);

	private:
		friend class Engine;
		GameTime(Engine* pEngine);

		Engine* m_pEngine;
		float m_TimeScale = 1.0f;
		int m_TotalFrames = 0;

		double m_DeltaTime = 0.0f;
		double m_LastTime = 0.0f;
		uint64_t m_StartTime = 0;
	};
}
