#include "ProfilerModule.h"

namespace Glory
{
	ProfilerModule::ProfilerModule() : m_SampleCollectingEnabled(false), m_IsRecording(false), m_CurrentSampleRecordSize(0), m_CurrentSampleWrite(-1)
	{
	}

	ProfilerModule::~ProfilerModule()
	{
	}

	void ProfilerModule::BeginThread(const std::string& name)
	{
		std::thread::id currentThreadID = std::this_thread::get_id();
		m_ThreadIDToProfile[currentThreadID] = name;
		m_CurrentThreadSamples.emplace(name, ProfilerThreadSample(name));
	}

	void ProfilerModule::EndThread()
	{
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		StoreSampleRecord(m_CurrentThreadSamples[threadName]);
		m_CurrentThreadSamples[threadName].Clear();
	}

	void ProfilerModule::BeginSample(const std::string& name)
	{
		if (!m_SampleCollectingEnabled) return;
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		m_CurrentThreadSamples[threadName].BeginSample(name);
	}

	void ProfilerModule::EndSample()
	{
		if (!m_SampleCollectingEnabled) return;
		std::thread::id currentThreadID = std::this_thread::get_id();
		std::string threadName = m_ThreadIDToProfile[currentThreadID];
		m_CurrentThreadSamples[threadName].EndSample();
	}

	void ProfilerModule::EnableSampleCollecting(bool enabled)
	{
		m_SampleCollectingEnabled = enabled;
	}

	void ProfilerModule::StartRecording()
	{
		m_CurrentSampleRecordSize = 0;
		m_IsRecording = true;
	}

	void ProfilerModule::EndRecording()
	{
		m_IsRecording = false;
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

	void ProfilerModule::StoreSampleRecord(const ProfilerThreadSample& sample)
	{
		if (!m_IsRecording) return;
		//std::unique_lock<std::mutex> lock(m_RecordMutex);

		++m_CurrentSampleWrite;
		if (m_CurrentSampleWrite >= MAX_SAMPLE_RECORDS) m_CurrentSampleWrite = 0;
		if (m_CurrentSampleRecordSize < MAX_SAMPLE_RECORDS) ++m_CurrentSampleRecordSize;
		m_SampleRecords[m_CurrentSampleWrite].m_Name = sample.m_Name;
		m_SampleRecords[m_CurrentSampleWrite].m_Samples = sample.m_Samples;

		//lock.unlock();
	}
}
