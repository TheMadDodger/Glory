#pragma once
#include "Thread.h"
#include <vector>

namespace Glory
{
	class Engine;

	class GameThread
	{
	public:
		GameThread(Engine* pEngine);
		virtual ~GameThread();

		void Start();
		void Stop();

		template<typename T>
		void Bind(T* pModule)
		{
			m_TickBinds.push_back(std::bind(&T::Tick, pModule));
			m_DrawBinds.push_back(std::bind(&T::Paint, pModule));
		}

	private:
		void Run();

		void Tick();
		void Paint();

	private:
		Thread* m_pThread;
		Engine* m_pEngine;
		std::vector<std::function<void()>> m_TickBinds;
		std::vector<std::function<void()>> m_DrawBinds;
		bool m_Exit;
		std::mutex m_ExitMutex;
	};
}
