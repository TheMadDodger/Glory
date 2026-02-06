#include "JobManager.h"

namespace Glory::Jobs
{
	JobManager::JobManager(ThreadManager* pThreads): m_pThreads(pThreads)
	{
	}

	JobManager::~JobManager()
	{
		for (size_t i = 0; i < m_pJobPools.size(); i++)
		{
			delete m_pJobPools[i];
		}
		m_pJobPools.clear();

		m_pThreads = nullptr;
	}
	
	void JobManager::Kill()
	{
		for (size_t i = 0; i < m_pJobPools.size(); i++)
		{
			m_pJobPools[i]->Kill();
		}
	}

	ThreadManager* JobManager::Threads() const
	{
		return m_pThreads;
	}
}