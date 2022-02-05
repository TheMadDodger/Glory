#include "GraphicsThread.h"
#include "ThreadManager.h"
#include "Game.h"
#include "Engine.h"
#include "FrameStates.h"

namespace Glory
{
	GraphicsThread::GraphicsThread(Engine* pEngine) : m_pRenderQueue(nullptr), m_pThread(nullptr), m_Exit(false), m_pEngine(pEngine)
	{
	}

	GraphicsThread::~GraphicsThread()
	{
		m_RenderBinds.clear();
		m_InitializationBinds.clear();
		m_CleanupBinds.clear();

		m_pThread = nullptr;

		delete m_pRenderQueue;
		m_pRenderQueue = nullptr;
	}

	void GraphicsThread::Start()
	{
		m_pRenderQueue = new RenderQueue(2);
		m_pThread = ThreadManager::Run(std::bind(&GraphicsThread::Run, this));
	}

	void GraphicsThread::Stop()
	{
		// Kill the thread
		m_Exit = true;
		m_pRenderQueue->Stop();
	}

	RenderQueue* GraphicsThread::GetRenderQueue()
	{
		return m_pRenderQueue;
	}

	void GraphicsThread::Run()
	{
		for (size_t i = 0; i < m_InitializationBinds.size(); i++)
		{
			m_InitializationBinds[i]();
		}

		while (true)
		{
			if (m_Exit) break;
			m_pRenderQueue->GetNextFrame(
				[&](const RenderFrame& frame)
				{
					OnRenderFrame(frame);
				});
		}

		for (size_t i = 0; i < m_CleanupBinds.size(); i++)
		{
			m_CleanupBinds[i]();
		}
	}

	void GraphicsThread::OnRenderFrame(const RenderFrame& frame)
	{
		for (size_t i = 0; i < m_BeginRenderBinds.size(); i++) m_BeginRenderBinds[i]();
		m_pEngine->GraphicsThreadFrameStart();
		FrameStates* pFrameStates = Game::GetGame().GetEngine()->GetGraphicsModule()->GetFrameStates();
		pFrameStates->OnFrameStart();
		for (size_t i = 0; i < m_RenderBinds.size(); i++)
		{
			m_RenderBinds[i](frame);
		}
		pFrameStates->OnFrameEnd();
		m_pEngine->GraphicsThreadFrameEnd();
		for (size_t i = 0; i < m_EndRenderBinds.size(); i++) m_EndRenderBinds[i]();
	}
}
