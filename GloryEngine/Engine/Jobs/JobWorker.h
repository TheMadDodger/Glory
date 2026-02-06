#pragma once
#include "JobQueue.h"

#include <functional>
#include <tuple>

namespace Glory::Jobs
{
	class JobWorkerPoolBase;

	/** @brief Helper method for starting a worker
	 * @param pPool The pool the worker belongs to
	 * @param func The workers main function
	 */
	void Run(JobWorkerPoolBase* pPool, std::function<void()> func);

	template<typename ret, typename ...args>
	class JobWorkerPool;

	/** @brief Job worker */
	template<typename ret, typename ...args>
	class JobWorker
	{
	public:
		/** @brief Constructor */
		JobWorker():
			m_pPool(nullptr), m_pJobQueue(nullptr), m_Exit(false), m_Running(false) {}
		/** @brief Destructor */
		virtual ~JobWorker() { m_pPool = nullptr; m_pJobQueue = nullptr; }

		/** @brief Initialize the worker by starting its thread
		 * @param pPool The pool this worker belongs to
		 * @param pJobQueue The queue this worker gets its jobs from
		 */
		void Initialize(JobWorkerPool<ret, args...>* pPool, JobQueue<ret, args...>* pJobQueue)
		{
			m_pPool = pPool;
			m_pJobQueue = pJobQueue;

			std::function<void()> func = std::bind(&JobWorker::JobThread, this);
			Run(m_pPool, func);
		}

		/** @brief Kill this worker */
		void Kill()
		{
			std::unique_lock<std::mutex> lock(m_ExitMutex);
			m_Exit = true;
			lock.unlock();
		}

		/** @brief Check whether this worker is running */
		bool IsRunning()
		{
			return m_Running;
		}

		/** @brief Default main function for a worker */
		void JobThread()
		{
			m_Running = true;
			while (true)
			{
				std::pair<std::function<ret(args...)>, std::tuple<args...>> job = m_pJobQueue->GetNextJob(m_Exit);
				std::unique_lock<std::mutex> lock(m_ExitMutex);
				if (m_Exit)
				{
					lock.unlock();
					m_Running = false;
					return;
				}
				lock.unlock();

				m_CurrentJob = job.first;

				ret result;
				std::apply([&](auto &&... arguments) { result = m_CurrentJob(arguments...); }, job.second);
				m_CurrentJob = NULL;
			}
		}

		/** @brief Check whether the worker is idle */
		bool IsIdle() const
		{
			return m_CurrentJob != NULL;
		}

	private:
		JobWorkerPool<ret, args...>* m_pPool;
		JobQueue<ret, args...>* m_pJobQueue;
		std::function<ret(args...)> m_CurrentJob;
		std::mutex m_ExitMutex;
		bool m_Exit;
		bool m_Running;
	};
}
