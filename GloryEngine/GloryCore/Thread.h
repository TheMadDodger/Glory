#pragma once
#include <functional>
#include <queue>
#include <mutex>

namespace Glory
{
	class Thread
	{
	public:
		const size_t& GetThreadIndex();
		bool IsIdle() const;

	private:
		Thread(size_t index, std::function<void(Thread*)> idleCallback);
		virtual ~Thread();

	private:
		void Initialize();
		void Assign(std::function<void()> func);
		void ThreadLoop();
		void Kill();

	private:
		friend class ThreadManager;
		const size_t THREAD_INDEX;
		std::thread m_Thread;
		std::queue<std::function<void()>> m_FunctionQueue;
		std::function<void()> m_CurrentFunc;
		std::mutex m_QueueMutex;
		std::condition_variable m_QueueCondition;
		std::function<void(Thread*)> m_IdleCallback;

		bool m_Exit;
		bool m_Idle;
	};
}
