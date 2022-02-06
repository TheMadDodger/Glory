#pragma once
#include <functional>
#include <queue>
#include "Job.h"

namespace Glory::Jobs
{
	class JobPoolBase
	{
	protected:
		JobPoolBase(size_t poolID, size_t numJobsPerThread);
		virtual ~JobPoolBase();

		virtual void Kill() = 0;

	protected:
		friend class JobManager;
		const size_t m_PoolID;
		const size_t m_NumJobsPerThread;
	};

	template<typename ret, typename ...args>
	class JobPool : public JobPoolBase
	{
	public:
		void StartQueue() { m_pJobQueue->Lock(); }
		void QueueJob(std::function<ret(args...)> job, args... arguments) { m_pJobQueue->Push(job, arguments...); }
		void QueueSingleJob(std::function<ret(args...)> job, args... arguments)
		{
			m_pJobQueue->Lock();
			m_pJobQueue->Push(job, arguments...);
			m_pJobQueue->UnLock();
			m_pJobQueue->ManualNotifyOne();
		}
		void EndQueue() { m_pJobQueue->UnLock(); }
		bool HasTasksInQueue() { return m_pJobQueue->Size() > 0; }

	private:
		JobPool(size_t poolID, size_t numJobsPerThread = 1) : JobPoolBase(poolID, numJobsPerThread), m_pJobQueue(nullptr) {}
		virtual ~JobPool()
		{
			for (size_t i = 0; i < m_pJobs.size(); i++)
			{
				delete m_pJobs[i];
			}
			m_pJobs.clear();

			delete m_pJobQueue;
			m_pJobQueue = nullptr;
		}

		void Initialize()
		{
			m_pJobQueue = new JobQueue<ret, args...>();
			
			size_t hardwareThreads = ThreadManager::NumHardwareThread();
			for (size_t i = 0; i < hardwareThreads * m_NumJobsPerThread; i++)
			{
				Job<ret, args...>* pJob = new Job<ret, args...>(m_pJobQueue);
				m_pJobs.push_back(pJob);
				pJob->Initialize();
			}
		}

		virtual void Kill() override
		{
			for (size_t i = 0; i < m_pJobs.size(); i++)
			{
				m_pJobs[i]->Kill();
			}
			m_pJobQueue->ManualNotify();
		}

	private:
		friend class JobManager;
		std::vector<Job<ret, args...>*> m_pJobs;
		JobQueue<ret, args...>* m_pJobQueue;
	};
}
