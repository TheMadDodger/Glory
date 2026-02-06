#pragma once
#include "JobWorkerPool.h"

#include <vector>

namespace Glory
{
	class ThreadManager;
}

namespace Glory::Jobs
{
	/** @brief Job manager */
	class JobManager
	{
	public:
		/** @brief Constructor
		 * @param pThreads Thread manager to run jobs on
		 */
		JobManager(ThreadManager* pThreads);
		/** @brief Destructor */
		virtual ~JobManager();

		/** @brief Kill all job pools and wait for exit */
		void Kill();
		/** @brief Get the thread manager the jobs run on */
		ThreadManager* Threads() const;

		/** @brief Create a new job worker pool
		 * @param numJobsPerThread Number of workers per CPU thread
		 */
		template<typename ret, typename ...args>
		JobWorkerPool<ret, args...>* Run(size_t numJobsPerThread = 1)
		{
			return CreateJobPool<ret, args...>();
		}


	private:
		template<typename ret, typename ...args>
		JobWorkerPool<ret, args...>* CreateJobPool(size_t numJobsPerThread = 1)
		{
			JobWorkerPool<ret, args...>* newPool = new JobWorkerPool<ret, args...>(this, m_pJobPools.size(), numJobsPerThread);
			m_pJobPools.push_back(newPool);
			newPool->Initialize();
			return newPool;
		}

	private:
		friend class Engine;
		std::vector<JobWorkerPoolBase*> m_pJobPools;
		ThreadManager* m_pThreads;
	};
}
