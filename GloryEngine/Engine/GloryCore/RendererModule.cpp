#include "RendererModule.h"
#include "Engine.h"
#include "CameraManager.h"
#include <algorithm>
#include "DisplayManager.h"
#include "EngineProfiler.h"
#include "GloryContext.h"

namespace Glory
{
	RendererModule::RendererModule() : m_LastSubmittedObjectCount(0), m_LastSubmittedCameraCount(0), m_PickPos(0, 0)
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

	void RendererModule::Submit(CameraRef camera, RenderTexture* pTexture)
	{
	}

	void RendererModule::Submit(const PointLight& light)
	{
		Profiler::BeginSample("RendererModule::Submit(light)");
		m_CurrentPreparingFrame.ActiveLights.push_back(light);
		OnSubmit(light);
		Profiler::EndSample();
	}

	void RendererModule::OnGameThreadFrameStart()
	{
		REQUIRE_MODULE(m_pEngine, GraphicsModule, );
		REQUIRE_MODULE(m_pEngine, RendererModule, );
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		Profiler::BeginSample("RendererModule::StartFrame");
		m_CurrentPreparingFrame = RenderFrame();
		Profiler::EndSample();
	}

	void RendererModule::OnGameThreadFrameEnd()
	{
		REQUIRE_MODULE(m_pEngine, GraphicsModule, );
		REQUIRE_MODULE(m_pEngine, RendererModule, );
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		Profiler::BeginSample("RendererModule::EndFrame");
		m_pEngine->GetGraphicsThread()->GetRenderQueue()->EnqueueFrame(m_CurrentPreparingFrame);
		Profiler::EndSample();
	}

	size_t RendererModule::LastSubmittedObjectCount()
	{
		return m_LastSubmittedObjectCount;
	}

	size_t RendererModule::LastSubmittedCameraCount()
	{
		return m_LastSubmittedCameraCount;
	}

	void RendererModule::SetNextFramePick(const glm::ivec2& coord, CameraRef camera)
	{
		m_PickPos = coord;
		m_PickCamera = camera;
	}

	void RendererModule::Initialize()
	{
		REQUIRE_MODULE_MESSAGE(m_pEngine, WindowModule, "A renderer module was loaded but there is no WindowModule present to render to.", Warning, );
		REQUIRE_MODULE_MESSAGE(m_pEngine, GraphicsModule, "A renderer module was loaded but there is no GraphicsModule present.", Warning, );
	}

	void RendererModule::Render(const RenderFrame& frame)
	{
		ReadHoveringObject();

		Profiler::BeginSample("RendererModule::Render");
		DisplayManager::ClearAllDisplays(m_pEngine);

		for (size_t i = 0; i < frame.ActiveCameras.size(); i++)
		{
			CameraRef camera = frame.ActiveCameras[i];

			RenderTexture* pRenderTexture = GloryContext::GetCameraManager()->GetRenderTextureForCamera(camera, m_pEngine);
			pRenderTexture->Bind();
			m_pEngine->GetGraphicsModule()->Clear(camera.GetClearColor());

			OnStartCameraRender(camera, frame.ActiveLights);

			for (size_t j = 0; j < frame.ObjectsToRender.size(); j++)
			{
				LayerMask mask = camera.GetLayerMask();
				if (mask != 0 && (mask & frame.ObjectsToRender[j].m_LayerMask) == 0) continue;
				Profiler::BeginSample("RendererModule::OnRender");
				OnRender(camera, frame.ObjectsToRender[j]);
				Profiler::EndSample();
			}

			OnEndCameraRender(camera, frame.ActiveLights);
			pRenderTexture->UnBind();

			RenderTexture* pOutputTexture = camera.GetOutputTexture();
			if (camera.HasOutput())
			{
				const glm::uvec2& resolution = camera.GetResolution();
				if (pOutputTexture == nullptr)
				{
					pOutputTexture = DisplayManager::CreateOutputTexture(m_pEngine, resolution.x, resolution.y);
					camera.SetOutputTexture(pOutputTexture);
				}
				uint32_t width, height;
				pOutputTexture->GetDimensions(width, height);
				if (width != resolution.x || height != resolution.y) pOutputTexture->Resize(resolution.x, resolution.y);

				Profiler::BeginSample("RendererModule::OnRender > Output Rendering");
				pOutputTexture->Bind();
				OnDoScreenRender(camera, frame.ActiveLights, width, height, pRenderTexture);
				pOutputTexture->UnBind();
				Profiler::EndSample();
			}

			int displayIndex = camera.GetDisplayIndex();
			if (displayIndex == -1) continue;
			RenderTexture* pDisplayRenderTexture = DisplayManager::GetDisplayRenderTexture(displayIndex);
			if (pDisplayRenderTexture == nullptr) continue;

			Window* pWindow = m_pEngine->GetWindowModule()->GetMainWindow();
			
			int width, height;
			pWindow->GetDrawableSize(&width, &height);

			Profiler::BeginSample("RendererModule::OnRender > Display Rendering");
			pDisplayRenderTexture->Bind();
			OnDoScreenRender(camera, frame.ActiveLights, width, height, pRenderTexture);
			pDisplayRenderTexture->UnBind();
			Profiler::EndSample();
		}

		m_LastSubmittedObjectCount = frame.ObjectsToRender.size();
		m_LastSubmittedCameraCount = frame.ActiveCameras.size();
		Profiler::EndSample();
	}

	void RendererModule::ReadHoveringObject()
	{
		Profiler::BeginSample("RendererModule::Pick");
		RenderTexture* pRenderTexture = GloryContext::GetCameraManager()->GetRenderTextureForCamera(m_PickCamera, m_pEngine, false);
		if (pRenderTexture == nullptr) return;
		Texture* pTexture = pRenderTexture->GetTextureAttachment("object");
		if (pTexture == nullptr) return;
		uint32_t objectID = pRenderTexture->ReadPixel(m_PickPos);
		m_pEngine->GetScenesModule()->SetHoveringObject(objectID);
		Profiler::EndSample();
	}

	RenderTexture* RendererModule::CreateCameraRenderTexture(uint32_t width, uint32_t height)
	{
		GPUResourceManager* pResourceManager = m_pEngine->GetGraphicsModule()->GetResourceManager();
		RenderTextureCreateInfo createInfo(width, height, true);
		GetCameraRenderTextureAttachments(createInfo.Attachments);
		createInfo.Attachments.push_back(Attachment("object", PixelFormat::PF_RI, PixelFormat::PF_R32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, false));
		return pResourceManager->CreateRenderTexture(createInfo);
	}

	void RendererModule::GetCameraRenderTextureAttachments(std::vector<Attachment>& attachments)
	{
		attachments.push_back(Attachment("color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
	}

	void RendererModule::OnCameraResize(CameraRef camera) {}

	void RendererModule::ThreadedInitialize()
	{
		DisplayManager::Initialize(m_pEngine);
		OnThreadedInitialize();
	}

	void RendererModule::ThreadedCleanup()
	{
		OnThreadedCleanup();
	}
}
