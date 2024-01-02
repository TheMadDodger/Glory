#include "GraphicsThread.h"
#include "ThreadManager.h"
#include "Engine.h"
#include "FrameStates.h"
#include "WindowModule.h"
#include "RendererModule.h"

namespace Glory
{
	GraphicsThread::GraphicsThread(Engine* pEngine)
		: m_pRenderQueue(nullptr), m_pThread(nullptr), m_Exit(false), m_pEngine(pEngine), m_Initialized(false)
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
		REQUIRE_MODULE(m_pEngine, GraphicsModule, );
		REQUIRE_MODULE(m_pEngine, RendererModule, );
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		m_pRenderQueue = new RenderQueue(2);
		m_pThread = ThreadManager::Run(std::bind(&GraphicsThread::Run, this));
	}

	void GraphicsThread::Stop()
	{
		REQUIRE_MODULE(m_pEngine, GraphicsModule, );
		REQUIRE_MODULE(m_pEngine, RendererModule, );
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		// Kill the thread
		m_Exit = true;
		m_pRenderQueue->Stop();

		while (!m_pThread->IsIdle()) {}
	}

	RenderQueue* GraphicsThread::GetRenderQueue()
	{
		return m_pRenderQueue;
	}

	void GraphicsThread::Execute(std::function<void(void*)> func, void* pData)
	{
		UUID uuid = UUID();

		ExecuteData data = ExecuteData();
		data.m_Func = func;
		data.m_pData = pData;
		data.m_UUID = uuid;
		m_Executes.push_back(data);

		//while (m_Executes.Contains([&](const ExecuteData& otherData) { return otherData.m_UUID == uuid; })) {}
	}

	const bool GraphicsThread::IsInitialized() const
	{
		return m_Initialized;
	}

	void GraphicsThread::Run()
	{
		for (size_t i = 0; i < m_InitializationBinds.size(); i++)
		{
			m_InitializationBinds[i]();
		}
		m_Initialized = true;

		while (true)
		{
			m_Executes.ForEachClear([](const ExecuteData& data) {data.m_Func(data.m_pData); });

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
		FrameStates* pFrameStates = m_pEngine->GetMainModule<GraphicsModule>()->GetFrameStates();
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
