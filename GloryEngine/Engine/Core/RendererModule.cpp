#include "RendererModule.h"
#include "Engine.h"
#include "CameraManager.h"
#include "DisplayManager.h"
#include "EngineProfiler.h"
#include "Buffer.h"
#include "FileLoaderModule.h"
#include "WindowModule.h"
#include "SceneManager.h"
#include "GraphicsModule.h"
#include "GraphicsThread.h"
#include "Engine.h"
#include "InternalMaterial.h"
#include "InternalPipeline.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <algorithm>

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

	void RendererModule::Submit(RenderData&& renderData)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(renderData)" };
		const size_t index = m_CurrentPreparingFrame.ObjectsToRender.size();
		m_CurrentPreparingFrame.ObjectsToRender.push_back(std::move(renderData));
		OnSubmit(m_CurrentPreparingFrame.ObjectsToRender[index]);
	}

	void RendererModule::Submit(CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(camera)" };
		auto it = std::find_if(m_CurrentPreparingFrame.ActiveCameras.begin(), m_CurrentPreparingFrame.ActiveCameras.end(), [camera, this](const CameraRef& other)
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

	void RendererModule::Submit(CameraRef camera, RenderTexture* pTexture)
	{
	}

	void RendererModule::Submit(PointLight&& light)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(light)" };
		const size_t index = m_CurrentPreparingFrame.ActiveLights.count();
		m_CurrentPreparingFrame.ActiveLights.push_back(std::move(light));
		OnSubmit(m_CurrentPreparingFrame.ActiveLights[index]);
	}

	void RendererModule::OnGameThreadFrameStart()
	{
		REQUIRE_MODULE(m_pEngine, GraphicsModule, );
		REQUIRE_MODULE(m_pEngine, RendererModule, );
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::StartFrame" };
		m_CurrentPreparingFrame = RenderFrame{};

		/* Stall if the queue is full */
		while (m_pEngine->GetGraphicsThread()->GetRenderQueue()->IsFull()) {}
	}

	void RendererModule::OnGameThreadFrameEnd()
	{
		REQUIRE_MODULE(m_pEngine, GraphicsModule, );
		REQUIRE_MODULE(m_pEngine, RendererModule, );
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::EndFrame" };
		m_pEngine->GetGraphicsThread()->GetRenderQueue()->EnqueueFrame(std::move(m_CurrentPreparingFrame));
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

	void RendererModule::DrawLineCircle(const glm::mat4& transform, const glm::vec3& position, float radius, CircleUp up, const glm::vec4& color)
	{
		static constexpr float pi = glm::pi<float>();
		static const float res = 100;
		static const float fullRadius = 360;
		static const float segments = fullRadius / res;

		for (float deg = 0; deg < fullRadius; deg += segments)
		{
			float d1 = deg;
			float d2 = deg + segments;

			const float x1 = radius * glm::sin(glm::radians(d1));
			const float y1 = radius * glm::cos(glm::radians(d1));
			const float x2 = radius * glm::sin(glm::radians(d2));
			const float y2 = radius * glm::cos(glm::radians(d2));

			switch (up)
			{
			case Glory::RendererModule::x:
				DrawLine(transform, position + glm::vec3{ 0, x1, y1 }, position + glm::vec3{ 0, x2, y2 }, color);
				break;
			case Glory::RendererModule::y:
				DrawLine(transform, position + glm::vec3{ x1, 0, y1 }, position + glm::vec3{ x2, 0, y2 }, color);
				break;
			case Glory::RendererModule::z:
				DrawLine(transform, position + glm::vec3{ x1, y1, 0 }, position + glm::vec3{ x2, y2, 0 }, color);
				break;
			default:
				break;
			}
		}
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

	void RendererModule::DrawLineSphere(const glm::mat4& transform, const glm::vec3& position, float radius, const glm::vec4& color)
	{
		DrawLineCircle(transform, position, radius, CircleUp::y, color);
		DrawLineCircle(transform, position, radius, CircleUp::z, color);
	}

	void RendererModule::DrawLineShape(const glm::mat4& transform, const glm::vec3& position, const ShapeProperty& shape, const glm::vec4& color)
	{
		switch (shape.m_ShapeType)
		{
		case ShapeType::Sphere: {
			const Sphere* sphere = shape.ShapePointer<Sphere>();
			DrawLineSphere(transform, position, sphere->m_Radius, color);
			break;
		}
		case ShapeType::Box: {
			const Box* box = shape.ShapePointer<Box>();
			DrawLineBox(transform, position, box->m_Extends, color);
			break;
		}
		case ShapeType::Cylinder:

			break;
		case ShapeType::Capsule:

			break;
		case ShapeType::TaperedCapsule:

			break;
		default:
			break;
		}
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

		std::vector<FileData*> pShaderFiles = { pVert, pFrag };
		std::vector<ShaderType> shaderTypes = { ShaderType::ST_Vertex, ShaderType::ST_Fragment };

		m_pLinesPipelineData = new InternalPipeline(std::move(pShaderFiles), std::move(shaderTypes));
		m_pLinesPipelineData->SetPipelineType(PipelineType::PT_Unknown);
		m_pLinesMaterialData = new InternalMaterial(m_pLinesPipelineData);
	}

	void RendererModule::PostInitialize()
	{
		m_pEngine->GetGraphicsThread()->BindForDraw(this);
		OnPostInitialize();
	}

	void RendererModule::Render(const RenderFrame& frame)
	{
		ReadHoveringObject();

		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Render" };
		m_pEngine->GetDisplayManager().ClearAllDisplays(m_pEngine);

		for (size_t i = 0; i < frame.ActiveCameras.size(); i++)
		{
			CameraRef camera = frame.ActiveCameras[i];

			RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
			pRenderTexture->BindForDraw();
			m_pEngine->GetMainModule<GraphicsModule>()->Clear(camera.GetClearColor());

			OnStartCameraRender(camera, frame.ActiveLights);

			for (size_t j = 0; j < frame.ObjectsToRender.size(); j++)
			{
				LayerMask mask = camera.GetLayerMask();
				if (mask != 0 && (mask & frame.ObjectsToRender[j].m_LayerMask) == 0) continue;
				m_pEngine->Profiler().BeginSample("RendererModule::OnRender");
				OnRender(camera, frame.ObjectsToRender[j]);
				m_pEngine->Profiler().EndSample();
			}

			RenderLines(camera);
			OnEndCameraRender(camera, frame.ActiveLights);
			pRenderTexture->UnBindForDraw();
			OnRenderEffects(camera, pRenderTexture);

			RenderTexture* pOutputTexture = camera.GetOutputTexture();
			if (camera.HasOutput())
			{
				const glm::uvec2& resolution = camera.GetResolution();
				if (pOutputTexture == nullptr)
				{
					pOutputTexture = m_pEngine->GetDisplayManager().CreateOutputTexture(m_pEngine, resolution.x, resolution.y);
					camera.SetOutputTexture(pOutputTexture);
				}
				uint32_t width, height;
				pOutputTexture->GetDimensions(width, height);
				if (width != resolution.x || height != resolution.y) pOutputTexture->Resize(resolution.x, resolution.y);

				m_pEngine->Profiler().BeginSample("RendererModule::OnRender > Output Rendering");
				pOutputTexture->BindForDraw();
				OnDoScreenRender(camera, frame.ActiveLights, width, height, pRenderTexture);
				pOutputTexture->UnBindForDraw();
				m_pEngine->Profiler().EndSample();
			}

			int displayIndex = camera.GetDisplayIndex();
			if (displayIndex == -1) continue;
			RenderTexture* pDisplayRenderTexture = m_pEngine->GetDisplayManager().GetDisplayRenderTexture(displayIndex);
			if (pDisplayRenderTexture == nullptr) continue;

			Window* pWindow = m_pEngine->GetMainModule<WindowModule>()->GetMainWindow();
			
			int width, height;
			pWindow->GetDrawableSize(&width, &height);

			m_pEngine->Profiler().BeginSample("RendererModule::OnRender > Display Rendering");
			pDisplayRenderTexture->BindForDraw();
			OnDoScreenRender(camera, frame.ActiveLights, width, height, pRenderTexture);
			pDisplayRenderTexture->UnBindForDraw();
			m_pEngine->Profiler().EndSample();
		}

		m_LastSubmittedObjectCount = frame.ObjectsToRender.size();
		m_LastSubmittedCameraCount = frame.ActiveCameras.size();
	}

	void RendererModule::ReadHoveringObject()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Pick" };
		RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(m_PickCamera, m_pEngine, 0, false);
		if (pRenderTexture == nullptr) return;
		Texture* pTexture = pRenderTexture->GetTextureAttachment("object");
		if (pTexture == nullptr) return;

		struct ObjData
		{
			uint64_t SceneID;
			uint64_t ObjectID;
		};

		ObjData object;
		float depth;

		pRenderTexture->ReadColorPixel("object", m_PickPos, &object, DataType::DT_UInt);
		pRenderTexture->ReadDepthPixel(m_PickPos, &depth, DataType::DT_Float);
		m_pEngine->GetSceneManager()->SetHoveringObject(object.SceneID, object.ObjectID);

		const float z = depth*2.0f - 1.0f;
		uint32_t width, height;
		pRenderTexture->GetDimensions(width, height);
		const glm::vec2 coord = glm::vec2{ m_PickPos.x/(float)width, m_PickPos.y/(float)height };

		const glm::vec4 clipSpacePosition{ coord * 2.0f - 1.0f, z, 1.0f };
		const glm::mat4 projectionInverse = m_PickCamera.GetProjectionInverse();
		const glm::mat4 viewInverse = m_PickCamera.GetViewInverse();
		glm::vec4 viewSpacePosition = projectionInverse*clipSpacePosition;

		/* Perspective division */
		viewSpacePosition /= viewSpacePosition.w;
		const glm::vec4 worldSpacePosition = viewInverse*viewSpacePosition;
		m_pEngine->GetSceneManager()->SetHoveringPosition(worldSpacePosition);
	}

	void RendererModule::CreateLineBuffer()
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		if (!pGraphics) return;
		m_pLineBuffer = pGraphics->GetResourceManager()->CreateBuffer(sizeof(LineVertex) * MAX_LINE_VERTICES, BufferBindingTarget::B_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
		m_pLineMesh = pGraphics->GetResourceManager()->CreateMesh(MAX_LINE_VERTICES, 0, InputRate::Vertex, 0, sizeof(LineVertex), PrimitiveType::PT_Lines, { AttributeType::Float3, AttributeType::Float4 }, m_pLineBuffer, nullptr);
		m_pLinesMaterial = pGraphics->GetResourceManager()->CreateMaterial(m_pLinesMaterialData);
	}

	void RendererModule::RenderLines(CameraRef camera)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		if (!pGraphics || !m_LineVertexCount) return;

		m_pLineMesh->BindForDraw();
		m_pLineBuffer->Assign(m_pLineVertices);

		Material* pMaterial = pGraphics->UseMaterial(m_pLinesMaterialData);

		ObjectData object;
		object.Model = glm::identity<glm::mat4>();
		object.View = camera.GetView();
		object.Projection = camera.GetProjection();
		object.ObjectID = 0;

		pMaterial->SetProperties(m_pEngine);
		pMaterial->SetObjectData(object);

		pGraphics->DrawMesh(m_pLineMesh, 0, m_LineVertexCount);
		pGraphics->UseMaterial(nullptr);

		m_LineVertexCount = 0;
		m_pLineVertex = m_pLineVertices;
	}

	void RendererModule::CreateCameraRenderTextures(uint32_t width, uint32_t height, std::vector<RenderTexture*>& renderTextures)
	{
		GPUResourceManager* pResourceManager = m_pEngine->GetMainModule<GraphicsModule>()->GetResourceManager();
		std::vector<RenderTextureCreateInfo> renderTextureInfos;
		GetCameraRenderTextureInfos(renderTextureInfos);

		renderTextures.resize(renderTextureInfos.size());

		for (size_t i = 0; i < renderTextureInfos.size(); ++i)
		{
			renderTextureInfos[i].Width = width;
			renderTextureInfos[i].Height = height;
			renderTextures[i] = pResourceManager->CreateRenderTexture(renderTextureInfos[i]);
		}
	}

	void RendererModule::GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos)
	{
		infos.resize(1);
		infos[0].HasDepth = true;
		infos[0].Attachments.push_back({Attachment("object", PixelFormat::PF_RGBAI, PixelFormat::PF_R32G32B32A32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UInt, false)});
		infos[0].Attachments.push_back({ Attachment("color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color) });
	}

	void RendererModule::OnCameraResize(CameraRef camera) {}

	void RendererModule::OnCameraPerspectiveChanged(CameraRef camera) {}

	void RendererModule::Draw()
	{
		m_pEngine->GetDebug().SubmitLines(this, &m_pEngine->Time());
	}

	void RendererModule::ThreadedInitialize()
	{
		m_pEngine->GetDisplayManager().Initialize(m_pEngine);
		CreateLineBuffer();
		OnThreadedInitialize();
	}

	void RendererModule::ThreadedCleanup()
	{
		OnThreadedCleanup();
	}
}
