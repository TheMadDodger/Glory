#include "JobPool.h"
#include "ThreadManager.h"

namespace Glory::Jobs
{
	const size_t JobPoolBase::NumHardwareThreads() const
	{
		return ThreadManager::NumHardwareThread();
	}
}
