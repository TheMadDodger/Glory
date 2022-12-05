#include "EngineProfiler.h"
#include "Game.h"
#include "Engine.h"
#include "ProfilerModule.h"

#define PROFILER Game::GetGame().GetEngine()->GetProfilerModule()

namespace Glory
{
	void Profiler::BeginThread(const std::string& name)
	{
		PROFILER->BeginThread(name);
	}

	void Profiler::EndThread()
	{
		PROFILER->EndThread();
	}

	void Profiler::BeginSample(const std::string& name)
	{
		PROFILER->BeginSample(name);
	}

	void Profiler::EndSample()
	{
		PROFILER->EndSample();
	}

	Profiler::Profiler() {}

	Profiler::~Profiler() {}
}