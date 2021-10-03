#pragma once
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include "Thread.h"

namespace Glory
{
	class ThreadManager
	{
	public:
		static Thread* Run(std::function<void()> func);
		static const size_t& NumHardwareThread();

	private:
		void Destroy();

		Thread* CreateThread(std::function<void()> func);

	private:
		bool NewThread();
		Thread* PopIdleThread();

		void OnThreadIdle(Thread* pThread);

		static ThreadManager* GetInstance();

		ThreadManager();
		virtual ~ThreadManager();

	private:
		friend class Engine;
		size_t m_HardwareThreads;
		std::mutex m_IdleQueueMutex;
		std::queue<size_t> m_IdleThreads;
		std::vector<Thread*> m_pThreads;
		static ThreadManager* m_pInstance;
	};
}
