#include "JobQueue.h"

namespace Glory::Jobs
{
	//template<typename ret, typename ...args>
	//inline JobQueue<ret, args...>::JobQueue()
	//{
	//
	//}
	//
	//template<typename ret, typename ...args>
	//JobQueue<ret, args...>::~JobQueue()
	//{
	//
	//}
	//
	//template<typename ret, typename ...args>
	//void JobQueue<ret, args...>::Push(std::function<ret(args...)> func)
	//{
	//	std::unique_lock<std::mutex> lock(m_QueueMutex);
	//	m_Queue.push(func);
	//	lock.unlock();
	//	m_QueueCondition.notify_one();
	//}

	//template<typename ret, typename ...args>
	//void JobQueue<ret, args...>::PushMultiple(std::vector<std::function<ret(args...)>> funcs)
	//{
	//	std::unique_lock<std::mutex> lock(m_QueueMutex);
	//	for (size_t i = 0; i < funcs.size(); i++)
	//	{
	//		m_Queue.push(funcs[i]);
	//	}
	//	lock.unlock();
	//	m_QueueCondition.notify_all();
	//}

	//template<typename ret, typename ...args>
	//std::function<ret(args...)> JobQueue<ret, args...>::GetNextJob()
	//{
	//	std::unique_lock<std::mutex> lock(m_QueueMutex);
	//	m_QueueCondition.wait(lock, [&]() { return !m_Queue.empty(); });
	//	std::function<ret(args...)> func = m_Queue.front();
	//	m_Queue.pop();
	//	lock.unlock();
	//	return func;
	//}
}
