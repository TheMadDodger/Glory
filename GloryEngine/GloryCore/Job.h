#pragma once
#include <functional>
#include <tuple>
#include "JobQueue.h"
#include "ThreadManager.h"

namespace Glory::Jobs
{
	template<typename ret, typename ...args>
	class Job
	{
	public:
		Job(JobQueue<ret, args...>* pJobQueue) : m_pJobQueue(pJobQueue) {}
		virtual ~Job()
		{

		}

		void Initialize()
		{
			std::function<void()> func = std::bind(&Job::JobThread, this);
			ThreadManager::Run(func);
		}

		void Kill()
		{
			std::unique_lock<std::mutex> lock(m_ExitMutex);
			m_Exit = true;
			lock.unlock();
		}

		void JobThread()
		{
			while (true)
			{
				std::pair<std::function<ret(args...)>, std::tuple<args...>> job = m_pJobQueue->GetNextJob(m_Exit);
				std::unique_lock<std::mutex> lock(m_ExitMutex);
				if (m_Exit)
				{
					lock.unlock();
					return;
				}
				lock.unlock();

				m_CurrentJob = job.first;

				ret result;
				std::apply([&](auto &&... arguments) { result = m_CurrentJob(arguments...); }, job.second);
			}
		}

	private:
		JobQueue<ret, args...>* m_pJobQueue;
		std::function<ret(args...)> m_CurrentJob;
		std::mutex m_ExitMutex;
		bool m_Exit;
	};
}
