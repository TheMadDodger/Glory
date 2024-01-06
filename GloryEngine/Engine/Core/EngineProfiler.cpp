#include "EngineProfiler.h"
#include "Engine.h"
#include "ProfilerModule.h"

namespace Glory
{
	void EngineProfiler::BeginThread(const std::string& name)
	{
		m_pProfiler->BeginThread(name);
	}

	void EngineProfiler::EndThread()
	{
		m_pProfiler->EndThread();
	}

	void EngineProfiler::BeginSample(const std::string& name)
	{
		m_pProfiler->BeginSample(name);
	}

	void EngineProfiler::EndSample()
	{
		m_pProfiler->EndSample();
	}

	EngineProfiler::EngineProfiler(): m_pProfiler(nullptr) {}

	EngineProfiler::~EngineProfiler() {}

	ProfileSample::ProfileSample(EngineProfiler* pProfiler, const std::string& name): Profiler(pProfiler)
	{
		pProfiler->BeginSample(name);
	}

	ProfileSample::~ProfileSample()
	{
		Profiler->EndSample();
	}
}