#include "GraphicsThread.h"
#include "ThreadManager.h"
#include "Game.h"
#include "Engine.h"

namespace Glory
{
	GraphicsThread::GraphicsThread()
	{
	}

	GraphicsThread::~GraphicsThread()
	{
	}

	void GraphicsThread::Start()
	{
		ThreadManager::Run(std::bind(&GraphicsThread::Run, this));

		//m_RenderThread = std::thread();
		//m_RenderThread
	}

	void GraphicsThread::Run()
	{
		while (true)
		{

		}
	}
}