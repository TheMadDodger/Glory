#pragma once
#include <string>
#include <chrono>
#include <vector>

namespace Glory
{
	class ProfilerSample
	{
	public:
		ProfilerSample(const std::string& name);
		virtual ~ProfilerSample();

		const std::string& Name();
		size_t SubSampleCount();
		ProfilerSample* GetSubSample(size_t index);

		template<typename T, typename Ratio>
		const T GetDuration()
		{
			std::chrono::duration<T, Ratio> duration = m_SampleEnd - m_SampleStart;
			return duration.count();
		}

	private:
		void BeginSubSample(const std::string& name);
		void EndSample();

	private:
		friend class ProfilerThreadSample;
		std::string m_Name;

		std::chrono::time_point<std::chrono::steady_clock> m_SampleStart;
		std::chrono::time_point<std::chrono::steady_clock> m_SampleEnd;

		bool m_Complete;

		std::vector<ProfilerSample> m_SubSamples;
	};
}
