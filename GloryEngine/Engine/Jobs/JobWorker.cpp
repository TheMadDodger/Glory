#include "JobWorker.h"
#include "JobWorkerPool.h"
#include "JobManager.h"

#include <ThreadManager.h>

namespace Glory::Jobs
{
	void Run(JobWorkerPoolBase* pPool, std::function<void()> func)
	{
		pPool->Jobs()->Threads()->Run(func);
	}
}
