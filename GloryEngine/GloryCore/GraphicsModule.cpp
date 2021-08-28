#include "GraphicsModule.h"
#include "TestCommandHandler.h"
#include "FrameStates.h"
#include "GraphicsCommandLibrary.h"

namespace Glory
{
	GraphicsModule::GraphicsModule() : m_pGraphicsThread(nullptr)
	{
	}

	GraphicsModule::~GraphicsModule()
	{
		delete m_pGraphicsThread;
		delete m_pFrameStates;
	}

	const std::type_info& GraphicsModule::GetModuleType()
	{
		return typeid(GraphicsModule);
	}

	GraphicsThread* GraphicsModule::GetGraphicsThread()
	{
		return m_pGraphicsThread;
	}

	FrameStates* GraphicsModule::GetFrameStates()
	{
		return m_pFrameStates;
	}

	void GraphicsModule::StartFrame()
	{
		m_PreparingFrame = RenderFrame();
	}

	void GraphicsModule::EnqueueCommand(const std::any& data)
	{
		m_PreparingFrame.Enqueue(data);
	}


	void GraphicsModule::EndFrame()
	{
		m_pGraphicsThread->GetRenderQueue()->EnqueueFrame(m_PreparingFrame);
	}

	Buffer* GraphicsModule::CreateVertexBuffer(uint32_t bufferSize)
	{
		Buffer* pVertexBuffer = CreateVertexBuffer_Internal(bufferSize);
		// Store it somewhere
		return pVertexBuffer;
	}

	void GraphicsModule::RegisterCommands()
	{
	}

	FrameStates* GraphicsModule::CreateFrameStates()
	{
		return new FrameStates(this);
	}

	void GraphicsModule::Initialize()
	{
		RegisterCommands();
		m_pFrameStates = CreateFrameStates();

		m_pGraphicsThread = new GraphicsThread();
		m_pGraphicsThread->Start();

		OnInitialize();
		m_pFrameStates->Initialize();
	}

	void GraphicsModule::Cleanup()
	{
		m_pGraphicsThread->Stop();
		GraphicsCommands::Destroy();

		OnCleanup();
	}

	void GraphicsModule::Update()
	{
		OnUpdate();
	}

	void GraphicsModule::Draw()
	{
		OnDraw();
	}
}