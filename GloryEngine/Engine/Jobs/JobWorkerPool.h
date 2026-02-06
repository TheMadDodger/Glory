#pragma once
#include "JobWorker.h"

#include <functional>
#include <queue>

namespace Glory::Jobs
{
	class JobManager;

	/** @brief Job worker pool base */
	class JobWorkerPoolBase
	{
	public:
		/** @brief Job manager that created this pool */
		JobManager* Jobs() const;

	protected:
		JobWorkerPoolBase(JobManager* pJobs, size_t poolID, size_t numJobsPerThread)
			: m_pJobs(pJobs), m_PoolID(poolID), m_NumJobsPerThread(numJobsPerThread) { }
		virtual ~JobWorkerPoolBase() { m_pJobs = nullptr; }

		virtual void Kill() = 0;

		static const size_t NumHardwareThreads();

	protected:
		friend class JobManager;
		const size_t m_PoolID;
		const size_t m_NumJobsPerThread;
		JobManager* m_pJobs;
	};

	/** @brief Job worker pool */
	template<typename ret, typename ...args>
	class JobWorkerPool : public JobWorkerPoolBase
	{
	public:
		/** @brief Begin a job queue */
		void StartQueue() { m_JobQueue.Lock(); }
		/** @brief Queue a new job
		 * @param job The job to execute
		 * @param arguments Arguments to pass to the job when it gets executed
		 *
		 * Make sure to call StartQueue() before queueing jobs!
		 */
		void QueueJob(std::function<ret(args...)> job, args... arguments) { m_JobQueue.Push(job, arguments...); }

		/** @brief Queue a new job and execute it immediately
		 * @param job The job to execute
		 * @param arguments Arguments to pass to the job when it gets executed
		 *
		 * Does not require a call to StartQueue()
		 */
		void QueueSingleJob(std::function<ret(args...)> job, args... arguments)
		{
			m_JobQueue.Lock();
			m_JobQueue.Push(job, arguments...);
			m_JobQueue.UnLock();
			m_JobQueue.ManualNotifyOne();
		}
		/** @brief End a job queue */
		void EndQueue() { m_JobQueue.UnLock(); }
		/** @brief Check whether this pool has jobs in its queue */
		bool HasTasksInQueue() { return m_JobQueue.Size() > 0; }
		/** @brief Check whether this pool is idle */
		bool IsIdle()
		{
			for (size_t i = 0; i < m_Workers.size(); ++i)
			{
				if (!m_Workers[i]->IsIdle()) return false;
			}
			return true;
		}

	private:
		JobWorkerPool(JobManager* pJobs, size_t poolID, size_t numJobsPerThread = 1):
			JobWorkerPoolBase(pJobs, poolID, numJobsPerThread), m_Workers(NumHardwareThreads()*numJobsPerThread) {}
		virtual ~JobWorkerPool()
		{
			m_Workers.clear();
		}

		void Initialize()
		{
			for (auto& worker : m_Workers)
			{
				worker.Initialize(this, &m_JobQueue);
			}
		}

		virtual void Kill() override
		{
			for (size_t i = 0; i < m_Workers.size(); i++)
			{
				m_Workers[i].Kill();
			}
			m_JobQueue.ManualNotify();

			for (size_t i = 0; i < m_Workers.size(); i++)
			{
				while (m_Workers[i].IsRunning()) {};
			}
		}

	private:
		friend class JobManager;
		std::vector<JobWorker<ret, args...>> m_Workers;
		JobQueue<ret, args...> m_JobQueue;
	};
}
