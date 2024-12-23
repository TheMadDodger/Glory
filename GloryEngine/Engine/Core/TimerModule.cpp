#include "TimerModule.h"

namespace Glory
{
	void TimerModule::OnBeginFrame()
	{
		m_CurrentFrameStart = std::chrono::steady_clock::now();
	}

	void TimerModule::OnEndFrame()
	{
		m_LastFrameEnd = std::chrono::steady_clock::now();
		m_LastFrameStart = m_CurrentFrameStart;

		++m_TotalFrames;
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
