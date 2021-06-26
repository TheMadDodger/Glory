#pragma once
#include "JobPool.h"
#include <vector>

namespace Glory::Jobs
{
	class JobManager
	{
	public:
		template<typename ret, typename ...args>
		static JobPool<ret, args...>* Run(size_t numJobsPerThread = 1)
		{
			return m_pInstance->CreateJobPool<ret, args...>();
		}

	private:
		JobManager();
		virtual ~JobManager();

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
		static JobManager* m_pInstance;
	};
}
