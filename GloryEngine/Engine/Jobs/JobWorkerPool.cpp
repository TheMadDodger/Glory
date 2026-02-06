#include "JobWorkerPool.h"
#include "ThreadManager.h"

namespace Glory::Jobs
{
	const size_t JobWorkerPoolBase::NumHardwareThreads()
	{
		return ThreadManager::NumHardwareThread();
	}

	JobManager* JobWorkerPoolBase::Jobs() const
	{
		return m_pJobs;
	}
}
