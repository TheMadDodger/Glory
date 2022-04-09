#pragma once
#include "ProfilerSample.h"
#include <string>
#include <vector>

namespace Glory
{
	class ProfilerThreadSample
	{
	public:
		ProfilerThreadSample();
		ProfilerThreadSample(const std::string& name);
		virtual ~ProfilerThreadSample();

		const std::string& Name() const;
		size_t SampleCount() const;
		ProfilerSample* GetSample(size_t index);

		template<typename T, typename Ratio>
		const T GetDuration() const
		{
			std::chrono::duration<T, Ratio> duration = m_End - m_Start;
			return duration.count();
		}

	private:
		void BeginSample(const std::string& name);
		void EndSample();
		void Clear();

	private:
		friend class ProfilerModule;
		std::string m_Name;
		std::vector<ProfilerSample> m_Samples;
		std::chrono::time_point<std::chrono::steady_clock> m_Start;
		std::chrono::time_point<std::chrono::steady_clock> m_End;
	};
}