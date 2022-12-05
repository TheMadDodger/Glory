#include "Thread.h"

namespace Glory
{
	const size_t& Thread::GetThreadIndex()
	{
		return THREAD_INDEX;
	}

	bool Thread::IsIdle() const
	{
		return m_Idle;
	}

	Thread::Thread(size_t index, std::function<void(Thread*)> idleCallback) : THREAD_INDEX(index), m_Exit(false), m_IdleCallback(idleCallback), m_Idle(true)
	{
	}

	Thread::~Thread()
	{
	}

	void Thread::Assign(std::function<void()> func)
	{
		std::unique_lock<std::mutex> lock(m_QueueMutex);
		m_FunctionQueue.push(func);
		lock.unlock();
		m_QueueCondition.notify_one();
	}

	void Thread::Initialize()
	{
		//auto binder = std::bind(&Thread::ThreadLoop, nullptr);
		m_Thread = std::thread(&Thread::ThreadLoop, this);
	}

	void Thread::ThreadLoop()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_QueueCondition.wait(lock, [&]() {return !m_FunctionQueue.empty() || m_Exit; });

			if (m_Exit) break;
			m_Idle = false;

			m_CurrentFunc = m_FunctionQueue.front();
			m_FunctionQueue.pop();
			lock.unlock();

			m_CurrentFunc();

			lock.lock();
			if (m_FunctionQueue.empty()) m_IdleCallback(this);
			m_Idle = true;
			lock.unlock();
		}
	}

	void Thread::Kill()
	{
		std::unique_lock<std::mutex> lock(m_QueueMutex);
		m_Exit = true;
		lock.unlock();
		m_QueueCondition.notify_one();
		m_Thread.join();
	}
}