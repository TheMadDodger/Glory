#include "RendererModule.h"
#include "Engine.h"
#include "CameraManager.h"
#include "DisplayManager.h"
#include "EngineProfiler.h"
#include "GloryContext.h"
#include "Buffer.h"

#include <algorithm>
#include "FileLoaderModule.h"

namespace Glory
{
	RendererModule::RendererModule()
		: m_LastSubmittedObjectCount(0), m_LastSubmittedCameraCount(0), m_PickPos(0, 0), m_LineVertexCount(0),
		m_pLineBuffer(nullptr), m_pLineMesh(nullptr), m_pLinesMaterialData(nullptr), m_pLinesMaterial(nullptr), m_pLineVertex(nullptr), m_pLineVertices(nullptr)
	{
	}

	RendererModule::~RendererModule()
	{
		if (m_pLinesMaterialData) delete m_pLinesMaterialData;
		m_pLinesMaterialData = nullptr;
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

	void RendererModule::DrawLine(const glm::mat4& transform, const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color)
	{
		const glm::vec4 transfromedP1 = transform * glm::vec4(p1, 1.0f);
		const glm::vec4 transfromedP2 = transform * glm::vec4(p2, 1.0f);
		m_pLineVertex->Pos = { transfromedP1.x, transfromedP1.y, transfromedP1.z };
		m_pLineVertex->Color = color;
		++m_pLineVertex;
		m_pLineVertex->Pos = { transfromedP2.x, transfromedP2.y, transfromedP2.z };
		m_pLineVertex->Color = color;
		++m_pLineVertex;
		m_LineVertexCount += 2;
	}

	void RendererModule::DrawLineQuad(const glm::mat4& transform, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec4& color)
	{
		DrawLine(transform, p1, p2, color);
		DrawLine(transform, p2, p3, color);
		DrawLine(transform, p3, p4, color);
		DrawLine(transform, p4, p1, color);
	}

	void RendererModule::DrawLineBox(const glm::mat4& transform, const glm::vec3& position, const glm::vec3& extends, const glm::vec4& color)
	{
		const glm::vec3 topTopLeft = position + glm::vec3(-extends.x, extends.y, -extends.z);
		const glm::vec3 topTopRight = position + glm::vec3(extends.x, extends.y, -extends.z);
		const glm::vec3 topBottomRight = position + glm::vec3(extends.x, extends.y, extends.z);
		const glm::vec3 topBottomLeft = position + glm::vec3(-extends.x, extends.y, extends.z);

		const glm::vec3 bottomTopLeft = position + glm::vec3(-extends.x, -extends.y, -extends.z);
		const glm::vec3 bottomTopRight = position + glm::vec3(extends.x, -extends.y, -extends.z);
		const glm::vec3 bottomBottomRight = position + glm::vec3(extends.x, -extends.y, extends.z);
		const glm::vec3 bottomBottomLeft = position + glm::vec3(-extends.x, -extends.y, extends.z);

		DrawLineQuad(transform, topTopLeft, topTopRight, topBottomRight, topBottomLeft, color);
		DrawLineQuad(transform, bottomTopLeft, bottomTopRight, bottomBottomRight, bottomBottomLeft, color);

		DrawLineQuad(transform, topBottomLeft, topTopLeft, bottomTopLeft, bottomBottomLeft, color);
		DrawLineQuad(transform, topBottomRight, topTopRight, bottomTopRight, bottomBottomRight, color);
	}

	void RendererModule::Initialize()
	{
		REQUIRE_MODULE_MESSAGE(m_pEngine, WindowModule, "A renderer module was loaded but there is no WindowModule present to render to.", Warning, );
		REQUIRE_MODULE_MESSAGE(m_pEngine, GraphicsModule, "A renderer module was loaded but there is no GraphicsModule present.", Warning, );

		m_pLineVertices = new LineVertex[MAX_LINE_VERTICES];
		m_pLineVertex = m_pLineVertices;

		FileImportSettings importSettings;
		importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
		importSettings.AddNullTerminateAtEnd = true;

		/* Line Shaders */
		std::filesystem::path path;
		GetResourcePath("Shaders/Lines_Vert.shader", path);
		FileData* pVert = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load(path.string(), importSettings);
		GetResourcePath("Shaders/Lines_Frag.shader", path);
		FileData* pFrag = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load(path.string(), importSettings);

		std::vector<ShaderSourceData*> pShaderFiles = { new ShaderSourceData(ShaderType::ST_Vertex, pVert), new ShaderSourceData(ShaderType::ST_Fragment, pFrag) };
		m_pLinesMaterialData = new MaterialData(pShaderFiles);
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

			RenderLines(camera);

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

	void RendererModule::CreateLineBuffer()
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		if (!pGraphics) return;
		m_pLineBuffer = pGraphics->GetResourceManager()->CreateBuffer(sizeof(LineVertex) * MAX_LINE_VERTICES, BufferBindingTarget::B_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
		m_pLineMesh = pGraphics->GetResourceManager()->CreateMesh(MAX_LINE_VERTICES, 0, InputRate::Vertex, 0, sizeof(LineVertex), PrimitiveType::PT_Lines, { AttributeType::Float3, AttributeType::Float4 }, m_pLineBuffer, nullptr);
		m_pLinesMaterial = pGraphics->GetResourceManager()->CreateMaterial(m_pLinesMaterialData);
	}

	void RendererModule::RenderLines(CameraRef camera)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		if (!pGraphics) return;

		m_pLineMesh->Bind();
		m_pLineBuffer->Assign(m_pLineVertices);

		Material* pMaterial = pGraphics->UseMaterial(m_pLinesMaterialData);

		ObjectData object;
		object.Model = glm::identity<glm::mat4>();
		object.View = camera.GetView();
		object.Projection = camera.GetProjection();
		object.ObjectID = 0;

		pMaterial->SetProperties();
		pMaterial->SetObjectData(object);

		pGraphics->DrawMesh(m_pLineMesh, 0, m_LineVertexCount);
		pGraphics->UseMaterial(nullptr);

		m_LineVertexCount = 0;
		m_pLineVertex = m_pLineVertices;
	}

	RenderTexture* RendererModule::CreateCameraRenderTexture(uint32_t width, uint32_t height)
	{
		GPUResourceManager* pResourceManager = m_pEngine->GetGraphicsModule()->GetResourceManager();
		RenderTextureCreateInfo createInfo(width, height, true);
		createInfo.Attachments.push_back(Attachment("object", PixelFormat::PF_RI, PixelFormat::PF_R32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, false));
		GetCameraRenderTextureAttachments(createInfo.Attachments);
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
		CreateLineBuffer();
		OnThreadedInitialize();
	}

	void RendererModule::ThreadedCleanup()
	{
		OnThreadedCleanup();
	}
}
