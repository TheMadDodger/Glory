#include "TimerModule.h"

namespace Glory
{
	void TimerModule::OnGameThreadFrameStart()
	{
		m_CurrentGameThreadFrameStart = std::chrono::steady_clock::now();
	}

	void TimerModule::OnGameThreadFrameEnd()
	{
		m_LastGameThreadFrameEnd = std::chrono::steady_clock::now();
		m_LastGameThreadFrameStart = m_CurrentGameThreadFrameStart;

		++m_TotalGameFrames;
	}

	void TimerModule::OnGraphicsThreadFrameStart()
	{
		m_CurrentGraphicsThreadFrameStart = std::chrono::steady_clock::now();
	}

	void TimerModule::OnGraphicsThreadFrameEnd()
	{
		m_LastGraphicsThreadFrameEnd = std::chrono::steady_clock::now();
		m_LastGraphicsThreadFrameStart = m_CurrentGraphicsThreadFrameStart;

		++m_TotalGraphicsFrames;
	}

	float TimerModule::CalculateTime(bool unscaled)
	{
		std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> time = currentTime - m_AppStart;
		return std::chrono::duration_cast<seconds>(time).count() * (unscaled ? 1.0f : m_TimeScale);
	}

	TimerModule::TimerModule() : m_AppStart(std::chrono::steady_clock::now()) {}

	TimerModule::~TimerModule() {}

	const std::type_info& TimerModule::GetModuleType()
	{
		return typeid(TimerModule);
	}

	void TimerModule::Initialize() {}
	void TimerModule::Cleanup() {}
}
