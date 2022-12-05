#include "JobManager.h"

namespace Glory::Jobs
{
	JobManager* JobManager::m_pInstance = nullptr;

	JobManager::JobManager()
	{
		m_pInstance = this;
	}

	JobManager::~JobManager()
	{
		for (size_t i = 0; i < m_pJobPools.size(); i++)
		{
			delete m_pJobPools[i];
		}
		m_pJobPools.clear();
	}

	JobManager* JobManager::GetInstance()
	{
		if (m_pInstance != nullptr) return m_pInstance;
		m_pInstance = new JobManager();
		return m_pInstance;
	}
	
	void JobManager::Kill()
	{
		for (size_t i = 0; i < m_pJobPools.size(); i++)
		{
			m_pJobPools[i]->Kill();
		}
	}
}