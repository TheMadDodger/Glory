#include "JobPool.h"
#include "ThreadManager.h"

namespace Glory::Jobs
{
	JobPoolBase::JobPoolBase(size_t poolID, size_t numJobsPerThread) : m_PoolID(poolID), m_NumJobsPerThread(numJobsPerThread)
	{
	}

	JobPoolBase::~JobPoolBase()
	{
	}

	//template<typename ret, typename ...args>
	//inline JobPool<ret, args...>::JobPool(size_t poolID, size_t numJobsPerThread) : JobPoolBase(poolID, numJobsPerThread)
	//{
	//
	//}
	//
	//template<typename ret, typename ...args>
	//inline JobPool<ret, args...>::~JobPool()
	//{
	//
	//}
	//
	//template<typename ret, typename ...args>
	//inline void JobPool<ret, args...>::Initialize()
	//{
	//	m_pJobQueue = new JobQueue<ret, args...>();
	//
	//	size_t hardwareThreads = ThreadManager::NumHardwareThread();
	//	for (size_t i = 0; i < hardwareThreads * m_NumJobsPerThread; i++)
	//	{
	//		Job<ret, args...>* pJob = new Job<ret, args...>();
	//		m_pJobs.push_back(pJob);
	//		ThreadManager::Run(&pJob->JobThread);
	//	}
	//}
	//
	//template<typename ret, typename ...args>
	//inline void JobPool<ret, args...>::QueueJob(std::function<ret(args...)> job)
	//{
	//	m_pJobQueue.Push(job);
	//}
	//
	//template<typename ret, typename ...args>
	//inline void JobPool<ret, args...>::QueueJobs(std::vector<std::function<ret(args...)>> jobs)
	//{
	//	m_pJobQueue.PushMultiple(jobs);
	//}
}
