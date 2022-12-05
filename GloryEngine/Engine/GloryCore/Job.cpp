#include "Job.h"

namespace Glory::Jobs
{
	//template<typename ret, typename ...args>
	//inline Job<ret, args...>::Job(JobQueue<ret, args...>* pJobQueue) : m_pJobQueue(pJobQueue)
	//{
	//
	//}
	//
	//template<typename ret, typename ...args>
	//Job<ret, args...>::~Job()
	//{
	//}

	//template<typename ret, typename ...args>
	//inline void Job<ret, args...>::JobThread()
	//{
	//	while (true)
	//	{
	//		m_pJobQueue->GetNextJob();
	//
	//		ret result;
	//		std::apply([&](auto &&... arguments) { result = m_CurrentJob(arguments...); }, m_Args);
	//	}
	//}
}
