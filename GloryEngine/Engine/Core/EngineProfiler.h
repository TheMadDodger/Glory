#pragma once
#include <string>

namespace Glory
{
	class ProfilerModule;

	class EngineProfiler
	{
	public:
		void BeginThread(const std::string& name);
		void EndThread();
		void BeginSample(const std::string& name);
		void EndSample();

	private:
		EngineProfiler(ProfilerModule* pProfiler);
		virtual ~EngineProfiler();

	private:
		ProfilerModule* m_pProfiler;
	};

	struct ProfileSample
	{
		ProfileSample(ProfilerModule* pProfiler, const std::string& name);
		~ProfileSample();

		ProfilerModule* Profiler;
	};
}
