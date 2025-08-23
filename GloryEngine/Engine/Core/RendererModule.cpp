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
#include "Engine.h"
#include "InternalMaterial.h"
#include "InternalPipeline.h"
#include "GScene.h"
#include "AssetManager.h"
#include "CubemapData.h"
#include "MaterialManager.h"
#include "GPUTextureAtlas.h"
#include "Window.h"
#include "GraphicsDevice.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Glory
{
	RendererModule::RendererModule()
		: m_LastSubmittedObjectCount(0), m_LastSubmittedCameraCount(0), m_LineVertexCount(0),
		m_pLineBuffer(nullptr), m_pLineMesh(nullptr), m_pLinesMaterialData(nullptr),
		m_pLineVertex(nullptr), m_pLineVertices(nullptr), m_DisplaysDirty(false),
		m_RenderPasses(RP_Count), m_FrameData(size_t(MAX_LIGHTS))
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

	void RendererModule::SubmitStatic(RenderData&& renderData)
	{
		Resource* pMaterialResource = m_pEngine->GetAssetManager().FindResource(renderData.m_MaterialID);
		if (!pMaterialResource)
		{
			/* We'll have to process it some other time */
			m_ToProcessStaticRenderData.emplace_back(std::move(renderData));
			return;
		}

		MaterialData* pMaterial = static_cast<MaterialData*>(pMaterialResource);

		const UUID pipelineID = pMaterial->GetPipelineID();
		/* Can't render anything without a pipeline */
		if (!pipelineID) return;

		auto& iter = std::find_if(m_StaticPipelineRenderDatas.begin(), m_StaticPipelineRenderDatas.end(),
			[pipelineID](const PipelineBatch& data) { return data.m_PipelineID == pipelineID; });
		PipelineBatch& pipelineRenderData = iter == m_StaticPipelineRenderDatas.end() ?
			m_StaticPipelineRenderDatas.emplace_back(pipelineID) : *iter;

		auto meshIter = pipelineRenderData.m_Meshes.find(renderData.m_MeshID);
		if (meshIter == pipelineRenderData.m_Meshes.end())
		{
			meshIter = pipelineRenderData.m_Meshes.emplace(renderData.m_MeshID, PipelineMeshBatch{ renderData.m_MeshID }).first;
			pipelineRenderData.m_UniqueMeshOrder.push_back(renderData.m_MeshID);
		}

		meshIter->second.m_Worlds.emplace_back(renderData.m_World);
		meshIter->second.m_ObjectIDs.emplace_back(renderData.m_SceneID, renderData.m_ObjectID);

		uint32_t materialIndex = 0;
		auto materialIter = std::find(pipelineRenderData.m_UniqueMaterials.begin(), pipelineRenderData.m_UniqueMaterials.end(), renderData.m_MaterialID);
		if (materialIter == pipelineRenderData.m_UniqueMaterials.end())
		{
			materialIndex = pipelineRenderData.m_UniqueMaterials.size();
			pipelineRenderData.m_UniqueMaterials.emplace_back(renderData.m_MaterialID);
		}
		else materialIndex = materialIter - pipelineRenderData.m_UniqueMaterials.begin();
		meshIter->second.m_MaterialIndices.emplace_back(materialIndex);
		pipelineRenderData.m_Dirty = true;
	}

	void RendererModule::UpdateStatic(UUID pipelineID, UUID meshID, UUID objectID, glm::mat4 world)
	{
		auto& pipelineIter = std::find_if(m_StaticPipelineRenderDatas.begin(), m_StaticPipelineRenderDatas.end(),
			[pipelineID](const PipelineBatch& otherPipeline) { return otherPipeline.m_PipelineID == pipelineID; });
		if (pipelineIter == m_StaticPipelineRenderDatas.end()) return;

		auto& meshIter = pipelineIter->m_Meshes.find(meshID);
		if (meshIter == pipelineIter->m_Meshes.end()) return;

		PipelineMeshBatch& meshRenderData = meshIter->second;

		auto objectIter = std::find_if(meshRenderData.m_ObjectIDs.begin(), meshRenderData.m_ObjectIDs.end(),
			[objectID](const std::pair<UUID, UUID>& ids) { return ids.second == objectID; });
		if (objectIter == meshRenderData.m_ObjectIDs.end()) return;
		const size_t instanceID = objectIter - meshRenderData.m_ObjectIDs.begin();
		meshRenderData.m_Worlds[instanceID] = world;
	}

	void RendererModule::UnsubmitStatic(UUID pipelineID, UUID meshID, UUID objectID)
	{
		auto pipelineIter = std::find_if(m_StaticPipelineRenderDatas.begin(), m_StaticPipelineRenderDatas.end(),
			[pipelineID](const PipelineBatch& otherPipeline) { return otherPipeline.m_PipelineID == pipelineID; });
		if (pipelineIter == m_StaticPipelineRenderDatas.end()) return;

		auto& meshIter = pipelineIter->m_Meshes.find(meshID);
		if (meshIter == pipelineIter->m_Meshes.end()) return;

		PipelineMeshBatch& meshRenderData = meshIter->second;

		auto objectIter = std::find_if(meshRenderData.m_ObjectIDs.begin(), meshRenderData.m_ObjectIDs.end(),
			[objectID](const std::pair<UUID, UUID>& ids) { return ids.second == objectID; });
		if (objectIter == meshRenderData.m_ObjectIDs.end()) return;
		const size_t index = objectIter - meshRenderData.m_ObjectIDs.begin();

		meshRenderData.m_ObjectIDs.erase(objectIter);
		meshRenderData.m_MaterialIndices.erase(meshRenderData.m_MaterialIndices.begin() + index);
		pipelineIter->m_Dirty = true;
	}

	void RendererModule::SubmitDynamic(RenderData&& renderData)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(RenderData)" };

		Resource* pMaterialResource = m_pEngine->GetAssetManager().FindResource(renderData.m_MaterialID);
		if (!pMaterialResource) return;

		MaterialData* pMaterial = static_cast<MaterialData*>(pMaterialResource);

		const UUID pipelineID = pMaterial->GetPipelineID();
		/* Can't render anything without a pipeline */
		if (!pipelineID) return;

		auto& iter = std::find_if(m_DynamicPipelineRenderDatas.begin(), m_DynamicPipelineRenderDatas.end(),
			[pipelineID](const PipelineBatch& data) { return data.m_PipelineID == pipelineID; });
		PipelineBatch& pipelineRenderData = iter == m_DynamicPipelineRenderDatas.end() ?
			m_DynamicPipelineRenderDatas.emplace_back(pipelineID) : *iter;

		auto meshIter = pipelineRenderData.m_Meshes.find(renderData.m_MeshID);
		if (meshIter == pipelineRenderData.m_Meshes.end())
		{
			meshIter = pipelineRenderData.m_Meshes.emplace(renderData.m_MeshID, PipelineMeshBatch{ renderData.m_MeshID }).first;
			pipelineRenderData.m_UniqueMeshOrder.push_back(renderData.m_MeshID);
		}

		meshIter->second.m_Worlds.emplace_back(renderData.m_World);
		meshIter->second.m_ObjectIDs.emplace_back(renderData.m_SceneID, renderData.m_ObjectID);

		uint32_t materialIndex = 0;
		auto materialIter = std::find(pipelineRenderData.m_UniqueMaterials.begin(), pipelineRenderData.m_UniqueMaterials.end(), renderData.m_MaterialID);
		if (materialIter == pipelineRenderData.m_UniqueMaterials.end())
		{
			materialIndex = pipelineRenderData.m_UniqueMaterials.size();
			pipelineRenderData.m_UniqueMaterials.emplace_back(renderData.m_MaterialID);
		}
		else materialIndex = materialIter - pipelineRenderData.m_UniqueMaterials.begin();
		meshIter->second.m_MaterialIndices.emplace_back(materialIndex);
		pipelineRenderData.m_Dirty = true;

		OnSubmitDynamic(renderData);
	}

	void RendererModule::SubmitLate(RenderData&& renderData)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::SubmitLate(RenderData)" };

		Resource* pMaterialResource = m_pEngine->GetAssetManager().FindResource(renderData.m_MaterialID);
		if (!pMaterialResource) return;

		MaterialData* pMaterial = static_cast<MaterialData*>(pMaterialResource);

		const UUID pipelineID = pMaterial->GetPipelineID();
		/* Can't render anything without a pipeline */
		if (!pipelineID) return;

		auto& iter = std::find_if(m_DynamicLatePipelineRenderDatas.begin(), m_DynamicLatePipelineRenderDatas.end(),
			[pipelineID](const PipelineBatch& data) { return data.m_PipelineID == pipelineID; });
		PipelineBatch& pipelineRenderData = iter == m_DynamicLatePipelineRenderDatas.end() ?
			m_DynamicLatePipelineRenderDatas.emplace_back(pipelineID) : *iter;

		auto meshIter = pipelineRenderData.m_Meshes.find(renderData.m_MeshID);
		if (meshIter == pipelineRenderData.m_Meshes.end())
		{
			meshIter = pipelineRenderData.m_Meshes.emplace(renderData.m_MeshID, PipelineMeshBatch{ renderData.m_MeshID }).first;
			pipelineRenderData.m_UniqueMeshOrder.push_back(renderData.m_MeshID);
		}

		meshIter->second.m_Worlds.emplace_back(renderData.m_World);
		meshIter->second.m_ObjectIDs.emplace_back(renderData.m_SceneID, renderData.m_ObjectID);

		uint32_t materialIndex = 0;
		auto materialIter = std::find(pipelineRenderData.m_UniqueMaterials.begin(), pipelineRenderData.m_UniqueMaterials.end(), renderData.m_MaterialID);
		if (materialIter == pipelineRenderData.m_UniqueMaterials.end())
		{
			materialIndex = pipelineRenderData.m_UniqueMaterials.size();
			pipelineRenderData.m_UniqueMaterials.emplace_back(renderData.m_MaterialID);
		}
		else materialIndex = materialIter - pipelineRenderData.m_UniqueMaterials.begin();
		meshIter->second.m_MaterialIndices.emplace_back(materialIndex);
		pipelineRenderData.m_Dirty = true;

		OnSubmitDynamic(renderData);
	}

	void RendererModule::Submit(CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(camera)" };
		auto it = std::find_if(m_FrameData.ActiveCameras.begin(), m_FrameData.ActiveCameras.end(), [camera, this](const CameraRef& other)
		{
			return camera.GetPriority() < other.GetPriority();
		});

		if (it != m_FrameData.ActiveCameras.end())
		{
			m_FrameData.ActiveCameras.insert(it, camera);
			OnSubmit(camera);
			return;
		}

		if (camera.GetDisplayIndex() != -1)
		{
			/* Resize camera to display manager */
			uint32_t width, height;
			m_pEngine->GetDisplayManager().GetResolution(width, height);
			camera.SetResolution(width, height);
		}

		m_FrameData.ActiveCameras.push_back(camera);
		OnSubmit(camera);
	}

	size_t RendererModule::Submit(const glm::ivec2& pickPos, UUID cameraID)
	{
		const size_t index = m_FrameData.Picking.size();
		m_FrameData.Picking.push_back({ pickPos, cameraID });
		return index;
	}

	void RendererModule::Submit(CameraRef camera, RenderTexture* pTexture)
	{
	}

	void RendererModule::Submit(LightData&& light, glm::mat4&& lightSpace, UUID id)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(light)" };
		const size_t index = m_FrameData.ActiveLights.count();
		m_FrameData.ActiveLights.push_back(std::move(light));
		m_FrameData.LightSpaceTransforms.push_back(std::move(lightSpace));
		m_FrameData.ActiveLightIDs.push_back(id);
		OnSubmit(m_FrameData.ActiveLights[index]);
	}

	void RendererModule::OnBeginFrame()
	{
		REQUIRE_MODULE(m_pEngine, GraphicsModule, );
		REQUIRE_MODULE(m_pEngine, RendererModule, );
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::StartFrame" };
		m_FrameData.Reset();
		std::for_each(m_DynamicPipelineRenderDatas.begin(), m_DynamicPipelineRenderDatas.end(), [](PipelineBatch& batch) { batch.Reset(); });
		std::for_each(m_DynamicLatePipelineRenderDatas.begin(), m_DynamicLatePipelineRenderDatas.end(), [](PipelineBatch& batch) { batch.Reset(); });
	}

	size_t RendererModule::LastSubmittedObjectCount()
	{
		return m_LastSubmittedObjectCount;
	}

	size_t RendererModule::LastSubmittedCameraCount()
	{
		return m_LastSubmittedCameraCount;
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
		DrawLineCircle(transform, position, radius, CircleUp::x, color);
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

	bool RendererModule::PickResultValid(size_t index) const
	{
		return m_LastFramePickResults.size() > index;
	}

	bool RendererModule::PickResultIndex(UUID cameraID, size_t& index) const
	{
		auto iter = std::find_if(m_LastFramePickResults.begin(), m_LastFramePickResults.end(), [cameraID](const PickResult& result) {
			return result.m_CameraID == cameraID;
		});
		if (iter == m_LastFramePickResults.end()) return false;
		index = iter - m_LastFramePickResults.begin();
		return true;
	}

	const PickResult& RendererModule::GetPickResult(size_t index) const
	{
		return m_LastFramePickResults[index];
	}

	void RendererModule::GetPickResult(UUID cameraID, std::function<void(const PickResult&)> callback)
	{
		std::scoped_lock<std::mutex> lock(m_PickLock);
		size_t index;
		if (!PickResultIndex(cameraID, index)) return;
		callback(m_LastFramePickResults[index]);
	}

	void RendererModule::OnWindowResize(glm::uvec2 size)
	{
		m_pEngine->GetCameraManager().ResizeAllCameras(size);
		m_DisplaysDirty = true;
	}

	void RendererModule::AddRenderPass(RenderPassType type, RenderPass&& pass)
	{
		m_RenderPasses[type].push_back(std::move(pass));
	}

	void RendererModule::RemoveRenderPass(RenderPassType type, std::string_view name)
	{
		auto iter = std::find_if(m_RenderPasses[type].begin(), m_RenderPasses[type].end(), [name](const RenderPass& pass) {return pass.m_Name == name; });
		if (iter == m_RenderPasses[type].end()) return;
		m_RenderPasses[type].erase(iter);
	}

	void RendererModule::RenderOnBackBuffer(RenderTexture* pTexture)
	{
		REQUIRE_MODULE_CALL(m_pEngine, GraphicsModule, Blit(pTexture), );

		for (auto& pass : m_RenderPasses[RP_Postblit])
		{
			pass.m_Callback(0, this);
		}
	}

	GPUTextureAtlas* RendererModule::CreateGPUTextureAtlas(TextureCreateInfo&& textureInfo, bool depth)
	{
		GPUTextureAtlas& newAtlas = m_GPUTextureAtlases.emplace_back(std::move(textureInfo), m_pEngine, depth);
		newAtlas.Initialize();
		return &newAtlas;
	}

	void RendererModule::Reset()
	{
		m_StaticPipelineRenderDatas.clear();
		m_DynamicPipelineRenderDatas.clear();
		m_DynamicLatePipelineRenderDatas.clear();
		m_FrameData.Reset();
	}

	CameraRef RendererModule::GetActiveCamera(uint32_t cameraIndex) const
	{
		return m_FrameData.ActiveCameras[cameraIndex];
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

		/* Line rendering */
		const UUID linesPipeline = Settings().Value<uint64_t>("Lines Pipeline");
		m_pLinesMaterialData = new MaterialData();
		m_pLinesMaterialData->SetPipeline(linesPipeline);

		m_RenderPasses[RP_LateobjectPass].push_back(RenderPass{ "Line Pass", [this](uint32_t cameraIndex, RendererModule*) {
			RenderLines(m_FrameData.ActiveCameras[cameraIndex]);
		} });
	}

	void RendererModule::PostInitialize()
	{
		m_pEngine->GetDisplayManager().Initialize(m_pEngine);
		CreateLineBuffer();
		OnPostInitialize();
	}

	void RendererModule::Render()
	{
		if (m_DisplaysDirty)
		{
			int width, height;
			m_pEngine->GetMainModule<WindowModule>()->GetMainWindow()->GetDrawableSize(&width, &height);
			m_pEngine->GetDisplayManager().ResizeAllTextures(uint32_t(width), uint32_t(height));
			m_DisplaysDirty = false;
		}

		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		m_PickResults.clear();

		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Render" };
		m_pEngine->GetDisplayManager().ClearAllDisplays(m_pEngine);

		for (auto& pass : m_RenderPasses[RP_Prepass])
		{
			pass.m_Callback(0, this);
		}

		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
			pRenderTexture->BindForDraw();
			pGraphics->Clear(camera.GetClearColor());

			for (auto& pass : m_RenderPasses[RP_CameraPrepass])
			{
				pass.m_Callback(i, this);
			}

			OnStartCameraRender(camera, m_FrameData.ActiveLights);

			for (auto& pass : m_RenderPasses[RP_ObjectPass])
			{
				pass.m_Callback(i, this);
			}

			/* Picking */
			for (size_t j = 0; j < m_FrameData.Picking.size(); ++j)
			{
				const auto& picking = m_FrameData.Picking[j];
				if (picking.second != camera.GetUUID()) continue;
				DoPicking(picking.first, camera);
			}
			
			pRenderTexture->BindForDraw();
			for (auto& pass : m_RenderPasses[RP_LateobjectPass])
			{
				pass.m_Callback(i, this);
			}

			OnEndCameraRender(camera, m_FrameData.ActiveLights);
			pRenderTexture->UnBindForDraw();
			OnRenderEffects(camera, pRenderTexture);

			for (auto& pass : m_RenderPasses[RP_CameraPostpass])
			{
				pass.m_Callback(i, this);
			}
		}

		for (auto& pass : m_RenderPasses[RP_PreCompositePass])
		{
			pass.m_Callback(0, this);
		}

		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
			pRenderTexture->BindForDraw();
			for (auto& pass : m_RenderPasses[RP_CameraCompositePass])
			{
				pass.m_Callback(i, this);
			}
			pRenderTexture->UnBindForDraw();

			/* Copy to display */
			int displayIndex = camera.GetDisplayIndex();
			if (displayIndex == -1) continue;

			RenderTexture* pOutputTexture = camera.GetOutputTexture();
			uint32_t width, height;
			pOutputTexture->GetDimensions(width, height);

			RenderTexture* pDisplayRenderTexture = m_pEngine->GetDisplayManager().GetDisplayRenderTexture(displayIndex);
			if (pDisplayRenderTexture == nullptr) continue;
			m_pEngine->Profiler().BeginSample("RendererModule::OnRender > Blit to Display");
			pDisplayRenderTexture->BindForDraw();
			OnDisplayCopy(pOutputTexture, width, height);
			pDisplayRenderTexture->UnBindForDraw();
			m_pEngine->Profiler().EndSample();
		}

		for (auto& pass : m_RenderPasses[RP_PostCompositePass])
		{
			pass.m_Callback(0, this);
		}

		for (auto& pass : m_RenderPasses[RP_Postpass])
		{
			pass.m_Callback(0, this);
		}

		//m_LastSubmittedObjectCount = m_FrameData.ObjectsToRender.size();
		m_LastSubmittedCameraCount = m_FrameData.ActiveCameras.size();

		std::scoped_lock lock(m_PickLock);
		m_LastFramePickResults.resize(m_PickResults.size());
		std::memcpy(m_LastFramePickResults.data(), m_PickResults.data(), m_PickResults.size()*sizeof(PickResult));
	}

	void RendererModule::DoPicking(const glm::ivec2& pos, CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::DoPicking" };
		RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine, 0, false);
		if (pRenderTexture == nullptr) return;
		Texture* pTexture = pRenderTexture->GetTextureAttachment("object");
		if (pTexture == nullptr) return;

		struct ObjData
		{
			uint64_t SceneID;
			uint64_t ObjectID;
		};

		ObjData object;
		glm::vec4 normal;
		float depth;

		/* Read pixels */
		pRenderTexture->ReadColorPixel("object", pos, &object, DataType::DT_UInt);
		pRenderTexture->ReadColorPixel("Normal", pos, &normal, DataType::DT_Float);
		pRenderTexture->ReadDepthPixel(pos, &depth, DataType::DT_Float);

		/* Calculate position */
		const float z = depth * 2.0f - 1.0f;
		uint32_t width, height;
		pRenderTexture->GetDimensions(width, height);
		const glm::vec2 coord = glm::vec2{ pos.x / (float)width, pos.y / (float)height };

		const glm::vec4 clipSpacePosition{ coord * 2.0f - 1.0f, z, 1.0f };
		const glm::mat4 projectionInverse = camera.GetProjectionInverse();
		const glm::mat4 viewInverse = camera.GetViewInverse();
		glm::vec4 viewSpacePosition = projectionInverse * clipSpacePosition;

		/* Perspective division */
		viewSpacePosition /= viewSpacePosition.w;
		const glm::vec4 worldSpacePosition = viewInverse * viewSpacePosition;

		/* Calculate normal */
		normal = normal * 2.0f - 1.0f;

		/* Store results */
		if (m_PickResults.empty())
		{
			m_pEngine->GetSceneManager()->SetHoveringObject(object.SceneID, object.ObjectID);
			m_pEngine->GetSceneManager()->SetHoveringPosition(worldSpacePosition);
			m_pEngine->GetSceneManager()->SetHoveringNormal(normal);
		}
		m_PickResults.push_back({ camera.GetUUID(), SceneObjectRef(object.SceneID, object.ObjectID), worldSpacePosition, normal});
	}

	void RendererModule::CreateLineBuffer()
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		if (!pGraphics) return;
		//m_pLineBuffer = pGraphics->GetResourceManager()->CreateBuffer(sizeof(LineVertex) * MAX_LINE_VERTICES, BufferBindingTarget::B_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
		//m_pLineMesh = pGraphics->GetResourceManager()->CreateMesh(MAX_LINE_VERTICES, 0, InputRate::Vertex, 0, sizeof(LineVertex), PrimitiveType::PT_Lines, { AttributeType::Float3, AttributeType::Float4 }, m_pLineBuffer, nullptr);
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
		pGraphics->EnableDepthWrite(false);
		pGraphics->EnableDepthTest(false);
		pMaterial->SetProperties(m_pEngine);
		pMaterial->SetObjectData(object);

		pGraphics->DrawMesh(m_pLineMesh, 0, m_LineVertexCount);
		pGraphics->UseMaterial(nullptr);
		pGraphics->EnableDepthWrite(true);
		pGraphics->EnableDepthTest(true);

		m_LineVertexCount = 0;
		m_pLineVertex = m_pLineVertices;
	}

	void RendererModule::CreateCameraRenderTextures(uint32_t width, uint32_t height, std::vector<RenderTexture*>& renderTextures)
	{
		//GPUResourceManager* pResourceManager = m_pEngine->GetMainModule<GraphicsModule>()->GetResourceManager();
		//std::vector<RenderTextureCreateInfo> renderTextureInfos;
		//GetCameraRenderTextureInfos(renderTextureInfos);
		//
		//renderTextures.resize(renderTextureInfos.size());
		//
		//for (size_t i = 0; i < renderTextureInfos.size(); ++i)
		//{
		//	renderTextureInfos[i].Width = width;
		//	renderTextureInfos[i].Height = height;
		//	renderTextures[i] = pResourceManager->CreateRenderTexture(renderTextureInfos[i]);
		//}
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
		//m_pEngine->GetDebug().SubmitLines(this, &m_pEngine->Time());
		//Render();

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		/* Make sure every camera has a render pass */
		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			RenderPassHandle& renderPass = reinterpret_cast<UUID&>(camera.GetUserHandle("RenderPass"));
			if (renderPass) continue;
			RenderPassInfo renderPassInfo;
			renderPassInfo.RenderTextureInfo.Width = camera.GetResolution().x;
			renderPassInfo.RenderTextureInfo.Height = camera.GetResolution().y;
			renderPassInfo.RenderTextureInfo.HasDepth = true;
			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("object", PixelFormat::PF_RGBAI, PixelFormat::PF_R32G32B32A32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UInt, false));
			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Debug", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R16G16B16A16Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("AOBlurred", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Data", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
			renderPass = pDevice->CreateRenderPass(renderPassInfo);
		}

		pDevice->Begin();

		m_PickResults.clear();

		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Render" };

		for (auto& pass : m_RenderPasses[RP_Prepass])
		{
			pass.m_Callback(0, this);
		}

		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			RenderPassHandle& renderPass = reinterpret_cast<UUID&>(camera.GetUserHandle("RenderPass"));
			pDevice->BeginRenderPass(renderPass);

			//RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
			//pRenderTexture->BindForDraw();
			//pGraphics->Clear(camera.GetClearColor());

			for (auto& pass : m_RenderPasses[RP_CameraPrepass])
			{
				pass.m_Callback(i, this);
			}

			//OnStartCameraRender(camera, m_FrameData.ActiveLights);

			for (auto& pass : m_RenderPasses[RP_ObjectPass])
			{
				pass.m_Callback(i, this);
			}

			/* Picking */
			/*for (size_t j = 0; j < m_FrameData.Picking.size(); ++j)
			{
				const auto& picking = m_FrameData.Picking[j];
				if (picking.second != camera.GetUUID()) continue;
				DoPicking(picking.first, camera);
			}*/

			//pRenderTexture->BindForDraw();
			for (auto& pass : m_RenderPasses[RP_LateobjectPass])
			{
				pass.m_Callback(i, this);
			}

			/*OnEndCameraRender(camera, m_FrameData.ActiveLights);
			pRenderTexture->UnBindForDraw();
			OnRenderEffects(camera, pRenderTexture);*/

			for (auto& pass : m_RenderPasses[RP_CameraPostpass])
			{
				pass.m_Callback(i, this);
			}

			pDevice->EndRenderPass();
		}

		for (auto& pass : m_RenderPasses[RP_PreCompositePass])
		{
			pass.m_Callback(0, this);
		}

		//for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		//{
		//	CameraRef camera = m_FrameData.ActiveCameras[i];

		//	RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
		//	pRenderTexture->BindForDraw();
		//	for (auto& pass : m_RenderPasses[RP_CameraCompositePass])
		//	{
		//		pass.m_Callback(i, this);
		//	}
		//	pRenderTexture->UnBindForDraw();

		//	/* Copy to display */
		//	int displayIndex = camera.GetDisplayIndex();
		//	if (displayIndex == -1) continue;

		//	RenderTexture* pOutputTexture = camera.GetOutputTexture();
		//	uint32_t width, height;
		//	pOutputTexture->GetDimensions(width, height);

		//	RenderTexture* pDisplayRenderTexture = m_pEngine->GetDisplayManager().GetDisplayRenderTexture(displayIndex);
		//	if (pDisplayRenderTexture == nullptr) continue;
		//	m_pEngine->Profiler().BeginSample("RendererModule::OnRender > Blit to Display");
		//	pDisplayRenderTexture->BindForDraw();
		//	OnDisplayCopy(pOutputTexture, width, height);
		//	pDisplayRenderTexture->UnBindForDraw();
		//	m_pEngine->Profiler().EndSample();
		//}

		for (auto& pass : m_RenderPasses[RP_PostCompositePass])
		{
			pass.m_Callback(0, this);
		}

		for (auto& pass : m_RenderPasses[RP_Postpass])
		{
			pass.m_Callback(0, this);
		}

		pDevice->End();

		//m_LastSubmittedObjectCount = m_FrameData.ObjectsToRender.size();
		//m_LastSubmittedCameraCount = m_FrameData.ActiveCameras.size();

		//std::scoped_lock lock(m_PickLock);
		//m_LastFramePickResults.resize(m_PickResults.size());
		//std::memcpy(m_LastFramePickResults.data(), m_PickResults.data(), m_PickResults.size() * sizeof(PickResult));
	}

	void RendererModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterAssetReference<PipelineData>("Lines Pipeline", 19);
	}

	PipelineBatch::PipelineBatch(UUID pipeline) : m_PipelineID(pipeline),
		m_Dirty(false)
	{
	}

	PipelineBatch::~PipelineBatch()
	{
	}

	void PipelineBatch::Reset()
	{
		m_Meshes.clear();
		m_UniqueMeshOrder.clear();
		m_UniqueMaterials.clear();
		m_Dirty = true;
	}
}
