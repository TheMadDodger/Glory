#pragma once
#include <string>

namespace Glory
{
	class Profiler
	{
	public:
		static void BeginThread(const std::string& name);
		static void EndThread();
		static void BeginSample(const std::string& name);
		static void EndSample();

	private:
		Profiler();
		virtual ~Profiler();
	};

	struct ProfileSample
	{
		ProfileSample(const std::string& name);
		~ProfileSample();
	};
}
