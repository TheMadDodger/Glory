#include "GraphicsModule.h"
#include "FrameStates.h"

namespace Glory
{
	GraphicsModule::GraphicsModule() : m_pFrameStates(nullptr)
	{
	}

	GraphicsModule::~GraphicsModule()
	{
		delete m_pFrameStates;
	}

	const std::type_info& GraphicsModule::GetModuleType()
	{
		return typeid(GraphicsModule);
	}

	FrameStates* GraphicsModule::GetFrameStates()
	{
		return m_pFrameStates;
	}

	Buffer* GraphicsModule::CreateVertexBuffer(uint32_t bufferSize)
	{
		Buffer* pVertexBuffer = CreateVertexBuffer_Internal(bufferSize);
		// Store it somewhere
		return pVertexBuffer;
	}

	FrameStates* GraphicsModule::CreateFrameStates()
	{
		return new FrameStates(this);
	}

	void GraphicsModule::Initialize()
	{
		m_pFrameStates = CreateFrameStates();
		OnInitialize();
		m_pFrameStates->Initialize();
	}

	void GraphicsModule::Cleanup()
	{
		OnCleanup();
	}
}