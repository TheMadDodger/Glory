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

		const std::string& Name();
		size_t SampleCount();
		ProfilerSample* GetSample(size_t index);

	private:
		void BeginSample(const std::string& name);
		void EndSample();
		void Clear();

	private:
		friend class ProfilerModule;
		std::string m_Name;
		std::vector<ProfilerSample> m_Samples;
	};
}