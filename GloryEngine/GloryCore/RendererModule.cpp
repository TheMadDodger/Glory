#include "RendererModule.h"
#include "Engine.h"
#include "CameraManager.h"
#include <algorithm>
#include "DisplayManager.h"
#include "EngineProfiler.h"

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
		Profiler::BeginSample("RendererModule::Submit(renderData)");
		m_CurrentPreparingFrame.ObjectsToRender.push_back(renderData);
		OnSubmit(renderData);
		Profiler::EndSample();
	}

	void RendererModule::Submit(CameraRef camera)
	{
		Profiler::BeginSample("RendererModule::Submit(camera)");
		auto it = std::find_if(m_CurrentPreparingFrame.ActiveCameras.begin(), m_CurrentPreparingFrame.ActiveCameras.end(), [camera](const CameraRef& other)
		{
			return camera.GetPriority() < other.GetPriority();
		});

		if (it != m_CurrentPreparingFrame.ActiveCameras.end())
		{
			m_CurrentPreparingFrame.ActiveCameras.insert(it, camera);
			OnSubmit(camera);
			Profiler::EndSample();
			return;
		}

		m_CurrentPreparingFrame.ActiveCameras.push_back(camera);
		OnSubmit(camera);
		Profiler::EndSample();
	}

	void RendererModule::StartFrame()
	{
		Profiler::BeginSample("RendererModule::StartFrame");
		m_CurrentPreparingFrame = RenderFrame();
		Profiler::EndSample();
	}

	void RendererModule::EndFrame()
	{
		Profiler::BeginSample("RendererModule::EndFrame");
		m_pEngine->GetGraphicsThread()->GetRenderQueue()->EnqueueFrame(m_CurrentPreparingFrame);
		Profiler::EndSample();
	}

	void RendererModule::Render(const RenderFrame& frame)
	{
		Profiler::BeginSample("RendererModule::Render");
		DisplayManager::ClearAllDisplays(m_pEngine);

		for (size_t i = 0; i < frame.ActiveCameras.size(); i++)
		{
			CameraRef camera = frame.ActiveCameras[i];

			RenderTexture* pRenderTexture = CameraManager::GetRenderTextureForCamera(camera, m_pEngine);
			pRenderTexture->Bind();
			m_pEngine->GetGraphicsModule()->Clear(camera.GetClearColor());

			for (size_t j = 0; j < frame.ObjectsToRender.size(); j++)
			{
				LayerMask mask = camera.GetLayerMask();
				if (mask != 0 && (mask & frame.ObjectsToRender[j].m_LayerMask) == 0) continue;
				Profiler::BeginSample("RendererModule::OnRender");
				OnRender(camera, frame.ObjectsToRender[j]);
				Profiler::EndSample();
			}

			pRenderTexture->UnBind();

			int displayIndex = camera.GetDisplayIndex();
			if (displayIndex == -1) continue;
			RenderTexture* pDisplayRenderTexture = DisplayManager::GetDisplayRenderTexture(displayIndex);
			if (pDisplayRenderTexture == nullptr) continue;

			Profiler::BeginSample("RendererModule::OnRender > Display Rendering");
			pDisplayRenderTexture->Bind();
			OnDoScreenRender(pRenderTexture);
			pDisplayRenderTexture->UnBind();
			Profiler::EndSample();
		}
		Profiler::EndSample();
	}

	void RendererModule::ThreadedInitialize()
	{
		DisplayManager::Initialize(m_pEngine);
	}

	void RendererModule::ThreadedCleanup() {}
}
