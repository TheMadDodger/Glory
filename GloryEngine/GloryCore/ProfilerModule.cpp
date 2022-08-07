#include "ProfilerModule.h"

namespace Glory
{
	ProfilerModule::ProfilerModule() : m_SampleCollectingEnabled(false)
	{
	}

	ProfilerModule::~ProfilerModule()
	{
	}

	void ProfilerModule::RegisterRecordCallback(std::function<void(const ProfilerThreadSample&)> callback)
	{
		m_RecordCallback = callback;
	}

	void ProfilerModule::BeginThread(const std::string& name)
	{
		std::thread::id currentThreadID = std::this_thread::get_id();
		m_ThreadIDToProfile[currentThreadID] = name;
		m_CurrentThreadSamples.emplace(name, ProfilerThreadSample(name));
		m_CurrentThreadSamples[name].m_Start = std::chrono::steady_clock::now();
	}

	void ProfilerModule::EndThread()
	{
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		m_CurrentThreadSamples[threadName].m_End = std::chrono::steady_clock::now();
		if (m_RecordCallback) m_RecordCallback(m_CurrentThreadSamples[threadName]);
		m_CurrentThreadSamples[threadName].Clear();
	}

	void ProfilerModule::BeginSample(const std::string& name)
	{
		if (!m_SampleCollectingEnabled) return;
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		if (threadName == "") return;
		m_CurrentThreadSamples[threadName].BeginSample(name);
	}

	void ProfilerModule::EndSample()
	{
		if (!m_SampleCollectingEnabled) return;
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		if (threadName == "") return;
		m_CurrentThreadSamples[threadName].EndSample();
	}

	void ProfilerModule::EnableSampleCollecting(bool enabled)
	{
		m_SampleCollectingEnabled = enabled;
	}

	const std::type_info& ProfilerModule::GetModuleType()
	{
		return typeid(ProfilerModule);
	}

	void ProfilerModule::Initialize()
	{
	}

	void ProfilerModule::Cleanup()
	{
	}

	void ProfilerModule::OnGameThreadFrameStart()
	{
		BeginThread("Game");
	}

	void ProfilerModule::OnGameThreadFrameEnd()
	{
		EndThread();
	}

	void ProfilerModule::OnGraphicsThreadFrameStart()
	{
		BeginThread("Graphics");
	}

	void ProfilerModule::OnGraphicsThreadFrameEnd()
	{
		EndThread();
	}
}
