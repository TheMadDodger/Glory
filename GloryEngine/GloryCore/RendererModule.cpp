#include "RendererModule.h"
#include "Engine.h"
#include "CameraManager.h"
#include <algorithm>
#include "DisplayManager.h"

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

	void RendererModule::Submit(CameraRef camera)
	{
		auto it = std::find_if(m_CurrentPreparingFrame.ActiveCameras.begin(), m_CurrentPreparingFrame.ActiveCameras.end(), [camera](const CameraRef& other)
		{
			return camera.GetPriority() < other.GetPriority();
		});

		if (it != m_CurrentPreparingFrame.ActiveCameras.end())
		{
			m_CurrentPreparingFrame.ActiveCameras.insert(it, camera);
			OnSubmit(camera);
			return;
		}

		m_CurrentPreparingFrame.ActiveCameras.push_back(camera);
		OnSubmit(camera);
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
		DisplayManager::ClearAllDisplays(m_pEngine);

		for (size_t i = 0; i < frame.ActiveCameras.size(); i++)
		{
			CameraRef camera = frame.ActiveCameras[i];

			RenderTexture* pRenderTexture = CameraManager::GetRenderTextureForCamera(camera, m_pEngine);
			pRenderTexture->Bind();
			m_pEngine->GetGraphicsModule()->Clear(camera.GetClearColor());

			for (size_t j = 0; j < frame.ObjectsToRender.size(); j++)
			{
				OnRender(camera, frame.ObjectsToRender[j]);
			}

			pRenderTexture->UnBind();

			int displayIndex = camera.GetDisplayIndex();
			if (displayIndex == -1) continue;
			RenderTexture* pDisplayRenderTexture = DisplayManager::GetDisplayRenderTexture(displayIndex);
			if (pDisplayRenderTexture == nullptr) continue;

			pDisplayRenderTexture->Bind();
			OnDoScreenRender(pRenderTexture);
			pDisplayRenderTexture->UnBind();
		}
	}

	void RendererModule::ThreadedInitialize()
	{
		DisplayManager::Initialize(m_pEngine);
	}

	void RendererModule::ThreadedCleanup() {}
}
