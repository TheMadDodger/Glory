#include "RendererModule.h"
#include "Engine.h"
#include "CameraManager.h"
#include <algorithm>

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

	void RendererModule::Submit(Camera* pCamera)
	{
		m_CurrentPreparingFrame.ActiveCameras.push_back(pCamera);
		OnSubmit(pCamera);
	}

	void RendererModule::StartFrame()
	{
		m_CurrentPreparingFrame = RenderFrame();
	}

	void RendererModule::EndFrame()
	{
		m_pEngine->GetGraphicsThread()->GetRenderQueue()->EnqueueFrame(m_CurrentPreparingFrame);
	}

	void RendererModule::Render(const RenderFrame& frame)
	{
		for (size_t i = 0; i < frame.ActiveCameras.size(); i++)
		{
			RenderTexture* pRenderTexture = CameraManager::GetRenderTextureForCamera(frame.ActiveCameras[i], m_pEngine);
			pRenderTexture->Bind();
			m_pEngine->GetGraphicsModule()->Clear();

			for (size_t j = 0; j < frame.ObjectsToRender.size(); j++)
			{
				OnRender(frame.ActiveCameras[i], frame.ObjectsToRender[j]);
			}

			pRenderTexture->UnBind();
			OnFinalRender(pRenderTexture);
		}
	}

	void RendererModule::ThreadedInitialize() {}

	void RendererModule::ThreadedCleanup() {}
}
