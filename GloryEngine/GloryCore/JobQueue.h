#pragma once
#include <queue>
#include <functional>
#include <mutex>
#include <vector>

namespace Glory::Jobs
{
	template<typename ret, typename ...args>
	class JobQueue
	{
	public:
		JobQueue()
		{
			m_PushLock = std::unique_lock<std::mutex>(m_QueueMutex);
			m_PushLock.unlock();
		}
		virtual ~JobQueue() {}

		void ManualNotify()
		{
			m_QueueCondition.notify_all();
		}

		void Lock()
		{
			m_PushLock.lock();
		}

		void Push(std::function<ret(args...)> func, args... arguments)
		{
			if (!m_PushLock.owns_lock()) return;
			std::pair<std::function<ret(args...)>, std::tuple<args...>> job
				= std::pair<std::function<ret(args...)>, std::tuple<args...>>(func, std::tuple<args...>(arguments...));
			m_Queue.push(job);
		}

		void UnLock()
		{
			m_PushLock.unlock();
			m_QueueCondition.notify_all();
		}

		void PushMultiple(std::vector<std::function<ret(args...)>> funcs)
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			for (size_t i = 0; i < funcs.size(); i++)
			{
				m_Queue.push(funcs[i]);
			}
			lock.unlock();
			m_QueueCondition.notify_all();
		}

		std::pair<std::function<ret(args...)>, std::tuple<args...>> GetNextJob(bool &exit)
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_QueueCondition.wait(lock, [&]() { return !m_Queue.empty() || exit; });
			if (exit)
			{
				lock.unlock();
				return std::pair<std::function<ret(args...)>, std::tuple<args...>>();
			}
			std::pair<std::function<ret(args...)>, std::tuple<args...>> func = m_Queue.front();
			m_Queue.pop();
			lock.unlock();
			return func;
		}

	private:
		std::mutex m_QueueMutex;
		std::unique_lock<std::mutex> m_PushLock;
		std::condition_variable m_QueueCondition;
		std::queue<std::pair<std::function<ret(args...)>, std::tuple<args...>>> m_Queue;
	};
}
