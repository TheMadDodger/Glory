#include "GameThread.h"
#include "ThreadManager.h"
#include "Engine.h"

namespace Glory
{
	GameThread::GameThread(Engine* pEngine) : m_pEngine(pEngine), m_pThread(nullptr)
	{
	}

	GameThread::~GameThread()
	{
		m_TickBinds.clear();
		m_DrawBinds.clear();
		m_pThread = nullptr;
		m_pEngine = nullptr;
	}

	void GameThread::Start()
	{
		m_pThread = ThreadManager::Run(std::bind(&GameThread::Run, this));
	}

	void GameThread::Stop()
	{
		std::unique_lock<std::mutex> lock(m_ExitMutex);
		m_Exit = true;
		lock.unlock();
	}

	void GameThread::Run()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(m_ExitMutex);
			bool exit = m_Exit;
			lock.unlock();

			if (exit) break;

			if (m_pEngine->GetGraphicsThread()->GetRenderQueue()->IsFull()) continue;
			Tick();
			Paint();
		}

		int a = 0;
	}

	void GameThread::Tick()
	{
		for (size_t i = 0; i < m_TickBinds.size(); i++)
		{
			m_TickBinds[i]();
		}
	}

	void GameThread::Paint()
	{
		for (size_t i = 0; i < m_DrawBinds.size(); i++)
		{
			m_DrawBinds[i]();
		}
	}
}
