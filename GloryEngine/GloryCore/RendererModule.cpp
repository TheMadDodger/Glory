#include "RendererModule.h"
#include <algorithm>
#include "Engine.h"

namespace Glory
{
	RendererModule::RendererModule()
	{
	}

	RendererModule::~RendererModule()
	{
	}

	const std::type_info& RendererModule::GetModuleType()
	{
		return typeid(RendererModule);
	}

	void RendererModule::Submit(const RenderData& renderData)
	{
		m_CurrentPreparingFrame.ObjectsToRender.push_back(renderData);
		OnSubmit(renderData);
	}

	void RendererModule::StartFrame()
	{
		m_CurrentPreparingFrame = RenderFrame();
	}

	void RendererModule::EndFrame()
	{
		m_pEngine->GetGraphicsThread()->GetRenderQueue()->EnqueueFrame(m_CurrentPreparingFrame);
	}

	void RendererModule::ThreadedInitialize() {}

	void RendererModule::ThreadedCleanup() {}

	//void RendererModule::OnDraw()
	//{
	//	std::for_each(m_ObjectsToRender.begin(), m_ObjectsToRender.end(), [&](const RenderData& renderData)
	//	{
	//		// Send to graphics module
	//	});
	//
	//	m_ObjectsToRender.clear();
	//}
}
