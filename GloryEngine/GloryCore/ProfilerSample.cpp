#include "ProfilerSample.h"

namespace Glory
{
	ProfilerSample::ProfilerSample(const std::string& name)
		: m_Name(name), m_SampleStart(std::chrono::steady_clock::now()), m_SampleEnd(), m_Complete(false)
	{
	}

	ProfilerSample::~ProfilerSample()
	{
	}

	void ProfilerSample::BeginSubSample(const std::string& name)
	{
		// Check if last sub sample is complete
		if (m_SubSamples.size() > 0 && !m_SubSamples[m_SubSamples.size() - 1].m_Complete)
		{
			(&m_SubSamples[m_SubSamples.size() - 1])->BeginSubSample(name);
			return;
		}

		m_SubSamples.push_back(ProfilerSample(name));
	}

	void ProfilerSample::EndSample()
	{
		// Check if last sub sample is complete
		if (m_SubSamples.size() > 0 && !m_SubSamples[m_SubSamples.size() - 1].m_Complete)
		{
			(&m_SubSamples[m_SubSamples.size() - 1])->EndSample();
			return;
		}

		m_SampleEnd = std::chrono::steady_clock::now();
		m_Complete = true;
	}
}
