#include "ThreadManager.h"
#include <mutex>

namespace Glory
{
	size_t ThreadManager::m_HardwareThreads = std::thread::hardware_concurrency();

	ThreadManager::~ThreadManager()
	{
		Kill();
	}

	Thread* ThreadManager::Run(std::function<void()> func)
	{
		return CreateThread(func);
	}

	size_t ThreadManager::NumHardwareThread()
	{
		if (m_HardwareThreads == 0) return 4;
		return m_HardwareThreads;
	}

	void ThreadManager::Kill()
	{
		for (size_t i = 0; i < m_pThreads.size(); i++)
		{
			m_pThreads[i]->Kill();
			delete m_pThreads[i];
		}
		m_pThreads.clear();
	}

	Thread* ThreadManager::CreateThread(std::function<void()> func)
	{
		std::unique_lock<std::mutex> lock(m_IdleQueueMutex);
		if (m_IdleThreads.empty() && !NewThread()) return nullptr;
		Thread* idleThread = PopIdleThread();
		lock.unlock();
		idleThread->Assign(func);
		return idleThread;
	}

	bool ThreadManager::NewThread()
	{
		const size_t index = m_pThreads.size();

		std::function<void(Thread*)> func = std::bind(&ThreadManager::OnThreadIdle, this, std::placeholders::_1);
		Thread* newThread = new Thread(index, func);

		m_pThreads.push_back(newThread);
		m_IdleThreads.push(index);
		newThread->Initialize();

		return true;
	}

	Thread* ThreadManager::PopIdleThread()
	{
		if (m_IdleThreads.empty()) return nullptr;
		const size_t index = m_IdleThreads.front();
		m_IdleThreads.pop();
		return m_pThreads[index];
	}

	void ThreadManager::OnThreadIdle(Thread* pThread)
	{
		std::unique_lock<std::mutex> lock(m_IdleQueueMutex);
		m_IdleThreads.push(pThread->THREAD_INDEX);
		lock.unlock();
	}
}
