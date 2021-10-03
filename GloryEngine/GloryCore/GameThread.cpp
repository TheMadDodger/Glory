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

	}

	void GameThread::Start()
	{
		m_pThread = ThreadManager::Run(std::bind(&GameThread::Run, this));
	}

	void GameThread::Stop()
	{

	}

	void GameThread::Run()
	{
		while (true)
		{
			if (m_pEngine->GetGraphicsThread()->GetRenderQueue()->IsFull()) continue;
			Tick();
			Paint();
		}
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
