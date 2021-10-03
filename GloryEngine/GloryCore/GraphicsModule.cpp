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
		m_pFrameStates = nullptr;
	}

	const std::type_info& GraphicsModule::GetModuleType()
	{
		return typeid(GraphicsModule);
	}

	FrameStates* GraphicsModule::GetFrameStates()
	{
		return m_pFrameStates;
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