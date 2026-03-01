#include "EngineProfiler.h"

namespace Glory
{
	EngineProfiler::EngineProfiler(): m_SampleCollectingEnabled(false) {}

	EngineProfiler::~EngineProfiler() {}

	void EngineProfiler::RegisterRecordCallback(std::function<void(const ProfilerThreadSample&)> callback)
	{
		m_RecordCallback = callback;
	}

	void EngineProfiler::BeginThread(const std::string& name)
	{
		std::thread::id currentThreadID = std::this_thread::get_id();
		m_ThreadIDToProfile[currentThreadID] = name;
		m_CurrentThreadSamples.emplace(name, ProfilerThreadSample(name));
		m_CurrentThreadSamples[name].m_Start = std::chrono::steady_clock::now();
	}

	void EngineProfiler::EndThread()
	{
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		m_CurrentThreadSamples[threadName].m_End = std::chrono::steady_clock::now();
		if (m_RecordCallback) m_RecordCallback(m_CurrentThreadSamples[threadName]);
		m_CurrentThreadSamples[threadName].Clear();
	}

	void EngineProfiler::BeginSample(const std::string& name)
	{
		if (!m_SampleCollectingEnabled) return;
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		if (threadName == "") return;
		m_CurrentThreadSamples[threadName].BeginSample(name);
	}

	void EngineProfiler::EndSample()
	{
		if (!m_SampleCollectingEnabled) return;
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		if (threadName == "") return;
		m_CurrentThreadSamples[threadName].EndSample();
	}

	void EngineProfiler::EnableSampleCollecting(bool enabled)
	{
		m_SampleCollectingEnabled = enabled;
	}

	ProfileSample::ProfileSample(EngineProfiler* pProfiler, const std::string& name): Profiler(pProfiler)
	{
		pProfiler->BeginSample(name);
	}

	ProfileSample::~ProfileSample()
	{
		Profiler->EndSample();
	}
}