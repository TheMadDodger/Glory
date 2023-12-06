#pragma once
#include "JobPool.h"
#include <vector>

namespace Glory::Jobs
{
	class JobManager
	{
	public:
		JobManager();
		virtual ~JobManager();

		template<typename ret, typename ...args>
		JobPool<ret, args...>* Run(size_t numJobsPerThread = 1)
		{
			return CreateJobPool<ret, args...>();
		}

	private:
		void Kill();

		template<typename ret, typename ...args>
		JobPool<ret, args...>* CreateJobPool(size_t numJobsPerThread = 1)
		{
			JobPool<ret, args...>* newPool = new JobPool<ret, args...>(m_pJobPools.size(), numJobsPerThread);
			m_pJobPools.push_back(newPool);
			newPool->Initialize();
			return newPool;
		}

	private:
		friend class Engine;
		std::vector<JobPoolBase*> m_pJobPools;
	};
}
