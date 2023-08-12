#include "Job.h"
#include "ThreadManager.h"

namespace Glory::Jobs
{
	void Run(std::function<void()> func)
	{
		ThreadManager::Run(func);
	}
}
