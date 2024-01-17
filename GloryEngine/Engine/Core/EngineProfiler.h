#pragma once
#include <string>

namespace Glory
{
	class ProfilerModule;

	class EngineProfiler
	{
	public:
		virtual ~EngineProfiler();

		void BeginThread(const std::string& name);
		void EndThread();
		void BeginSample(const std::string& name);
		void EndSample();

	private:
		EngineProfiler();

	private:
		friend class Engine;
		ProfilerModule* m_pProfiler;
	};

	struct ProfileSample
	{
		ProfileSample(EngineProfiler* pProfiler, const std::string& name);
		~ProfileSample();

		EngineProfiler* Profiler;
	};
}
