#include "ProfilerThreadSample.h"
#include "Debug.h"

namespace Glory
{
	ProfilerThreadSample::ProfilerThreadSample() : m_Name(""), m_Start(std::chrono::steady_clock::now())
	{
	}

	ProfilerThreadSample::ProfilerThreadSample(const std::string& name) : m_Name(name), m_Start(std::chrono::steady_clock::now())
	{
	}

	ProfilerThreadSample::~ProfilerThreadSample()
	{
	}

	const std::string& ProfilerThreadSample::Name() const
	{
		return m_Name;
	}

	size_t ProfilerThreadSample::SampleCount() const
	{
		return m_Samples.size();
	}

	ProfilerSample* ProfilerThreadSample::GetSample(size_t index)
	{
		return &m_Samples[index];
	}

	void ProfilerThreadSample::BeginSample(const std::string& name)
	{
		// Check if last sample is complete
		if (m_Samples.size() > 0 && !m_Samples[m_Samples.size() - 1].m_Complete)
		{
			(&m_Samples[m_Samples.size() - 1])->BeginSubSample(name);
			return;
		}

		m_Samples.push_back(ProfilerSample(name));
	}

	void ProfilerThreadSample::EndSample()
	{
		if (m_Samples.size() <= 0 || m_Samples[m_Samples.size() - 1].m_Complete)
		{
			//m_pEngine->GetDebug().LogError("Profiler: You are popping more samples than you are pushing! Make sure to start a sample with BeginSample.");
			return;
		}

		ProfilerSample* pSample = &m_Samples[m_Samples.size() - 1];
		pSample->EndSample();
	}

	void ProfilerThreadSample::Clear()
	{
		m_Samples.clear();
	}
}
