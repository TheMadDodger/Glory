#include "GraphicsThread.h"
#include "ThreadManager.h"
#include "Game.h"
#include "Engine.h"
#include "GraphicsCommandLibrary.h"
#include "FrameStates.h"

namespace Glory
{
	GraphicsThread::GraphicsThread() : m_pRenderQueue(nullptr), m_pThread(nullptr)
	{
	}

	GraphicsThread::~GraphicsThread()
	{
		delete m_pRenderQueue;
		m_pRenderQueue = nullptr;
	}

	void GraphicsThread::Start()
	{
		m_pRenderQueue = new RenderQueue();
		m_pThread = ThreadManager::Run(std::bind(&GraphicsThread::Run, this));
	}

	void GraphicsThread::Stop()
	{
		
	}

	RenderQueue* GraphicsThread::GetRenderQueue()
	{
		return m_pRenderQueue;
	}

	void GraphicsThread::Run()
	{
		while (true)
		{
			m_pRenderQueue->GetNextFrame([&](const RenderFrame& frame) { OnRenderFrame(frame); });
		}
	}

	void GraphicsThread::OnRenderFrame(const RenderFrame& frame)
	{
		// Tell the frame states a frame render started
		FrameStates* pFrameStates = Game::GetGame().GetEngine()->GetGraphicsModule()->GetFrameStates();
		pFrameStates->OnFrameStart();
		for (size_t i = 0; i < frame.CommandQueue.size(); ++i)
		{
			GraphicsCommands::RunCommand(frame.CommandQueue[i]);
		}
		pFrameStates->OnFrameEnd();
		// Tell the frame states a frame render finished
	}
}
