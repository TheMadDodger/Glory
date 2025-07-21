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
		GPUResourceManager* gpuResources = m_pEngine->GetMainModule<GraphicsModule>()->GetResourceManager();
		Resource* pMeshResource = m_pEngine->GetAssetManager().FindResource(renderData.m_MeshID);
		Resource* pMaterialResource = m_pEngine->GetAssetManager().FindResource(renderData.m_MaterialID);

		if (!pMeshResource || !pMaterialResource)
		{
			/* We'll have to process it some other time */
			m_ToProcessStaticRenderData.emplace_back(std::move(renderData));
			return;
		}

		MeshData* pMesh = static_cast<MeshData*>(pMeshResource);
		MaterialData* pMaterial = static_cast<MaterialData*>(pMaterialResource);

		const UUID pipelineID = pMaterial->GetPipelineID(m_pEngine->GetMaterialManager());
		/* Can't render anything without a pipeline */
		if (!pipelineID) return;

		auto& iter = std::find_if(m_StaticPipelineRenderDatas.begin(), m_StaticPipelineRenderDatas.end(),
			[pipelineID](const PipelineRenderData& data) { return data.m_Pipeline == pipelineID; });
		PipelineRenderData& pipelineRenderData = iter == m_StaticPipelineRenderDatas.end() ?
			m_StaticPipelineRenderDatas.emplace_back(pipelineID) : *iter;

		auto meshIter = pipelineRenderData.m_Meshes.find(renderData.m_MeshID);
		if (meshIter == pipelineRenderData.m_Meshes.end())
			meshIter = pipelineRenderData.m_Meshes.emplace(renderData.m_MeshID, PipelineMeshRenderData{ renderData.m_MeshID }).first;

		meshIter->second.m_Objects.emplace_back(PerObjectData{ renderData.m_SceneID, renderData.m_ObjectID, renderData.m_World });
		meshIter->second.m_Materials.emplace_back(renderData.m_MaterialID);
		pipelineRenderData.m_Dirty = true;
	}

	void RendererModule::UpdateStatic(UUID pipelineID, UUID meshID, UUID objectID, glm::mat4 world)
	{
		auto& pipelineIter = std::find_if(m_StaticPipelineRenderDatas.begin(), m_StaticPipelineRenderDatas.end(),
			[pipelineID](const PipelineRenderData& otherPipeline) { return otherPipeline.m_Pipeline == pipelineID; });
		if (pipelineIter == m_StaticPipelineRenderDatas.end()) return;

		auto& meshIter = pipelineIter->m_Meshes.find(meshID);
		if (meshIter == pipelineIter->m_Meshes.end()) return;

		PipelineMeshRenderData& meshRenderData = meshIter->second;

		auto objectIter = std::find_if(meshRenderData.m_Objects.begin(), meshRenderData.m_Objects.end(),
			[objectID](const PerObjectData& obj) { return obj.m_ObjectID == objectID; });
		if (objectIter == meshRenderData.m_Objects.end()) return;
		objectIter->m_World = world;
		
		auto& drawIDIter = std::find(pipelineIter->m_UniqueMeshOrder.begin(), pipelineIter->m_UniqueMeshOrder.end(), meshID);
		const size_t drawID = drawIDIter - pipelineIter->m_UniqueMeshOrder.begin();
		const size_t instanceID = objectIter - meshRenderData.m_Objects.begin();
		const size_t objectDataIndex = pipelineIter->m_ObjectDataOffsets.m_Data[drawID] + instanceID;
		pipelineIter->m_FinalPerObjectData.m_Data[objectDataIndex].m_World = world;
		pipelineIter->m_FinalPerObjectData.m_Dirty = true;
	}

	void RendererModule::UnsubmitStatic(UUID pipelineID, UUID meshID, UUID objectID)
	{
		auto pipelineIter = std::find_if(m_StaticPipelineRenderDatas.begin(), m_StaticPipelineRenderDatas.end(),
			[pipelineID](const PipelineRenderData& otherPipeline) { return otherPipeline.m_Pipeline == pipelineID; });
		if (pipelineIter == m_StaticPipelineRenderDatas.end()) return;

		auto& meshIter = pipelineIter->m_Meshes.find(meshID);
		if (meshIter == pipelineIter->m_Meshes.end()) return;

		PipelineMeshRenderData& meshRenderData = meshIter->second;

		auto objectIter = std::find_if(meshRenderData.m_Objects.begin(), meshRenderData.m_Objects.end(),
			[objectID](const PerObjectData& obj) { return obj.m_ObjectID == objectID; });
		if (objectIter == meshRenderData.m_Objects.end()) return;
		const size_t index = objectIter - meshRenderData.m_Objects.begin();

		meshRenderData.m_Objects.erase(objectIter);
		meshRenderData.m_Materials.erase(meshRenderData.m_Materials.begin() + index);
		pipelineIter->m_Dirty = true;
	}

	void RendererModule::SubmitDynamic(RenderData&& renderData)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(RenderData)" };
		const size_t index = m_FrameData.ObjectsToRender.size();
		m_FrameData.ObjectsToRender.push_back(std::move(renderData));
		OnSubmitDynamic(m_FrameData.ObjectsToRender[index]);
	}

	void RendererModule::SubmitDynamic(TextRenderData&& renderData)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(TextRenderData)" };
		const size_t index = m_FrameData.TextsToRender.size();
		m_FrameData.TextsToRender.push_back(std::move(renderData));
		OnSubmitDynamic(m_FrameData.TextsToRender[index]);
	}

	void RendererModule::SubmitLate(RenderData&& renderData)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::SubmitLate(RenderData)" };
		const size_t index = m_FrameData.ObjectsToRenderLate.size();
		m_FrameData.ObjectsToRenderLate.push_back(std::move(renderData));
		OnSubmitDynamic(m_FrameData.ObjectsToRenderLate[index]);
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

	void RendererModule::RenderObject(CameraRef camera, const RenderData& renderData)
	{
		OnRender(camera, renderData);
	}

	void RendererModule::RenderOnBackBuffer(RenderTexture* pTexture)
	{
		REQUIRE_MODULE_CALL(m_pEngine, GraphicsModule, Blit(pTexture), );

		for (auto& pass : m_RenderPasses[RP_Postblit])
		{
			pass.m_Callback(nullptr, {0});
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

		m_RenderPasses[RP_ObjectPass].push_back(RenderPass{ "Skybox Pass", [this](CameraRef camera, const RenderFrame& frame) {
			SkyboxPass(camera, frame);
		} });

		m_RenderPasses[RP_ObjectPass].push_back(RenderPass{ "Main Object Pass", [this](CameraRef camera, const RenderFrame& frame) {
			MainObjectPass(camera, frame);
		} });

		m_RenderPasses[RP_ObjectPass].push_back(RenderPass{ "Main Text Pass", [this](CameraRef camera, const RenderFrame& frame) {
			MainTextPass(camera, frame);
		} });

		m_RenderPasses[RP_LateobjectPass].push_back(RenderPass{ "Main Late Object Pass", [this](CameraRef camera, const RenderFrame& frame) {
			MainLateObjectPass(camera, frame);
		} });

		m_RenderPasses[RP_LateobjectPass].push_back(RenderPass{ "Line Pass", [this](CameraRef camera, const RenderFrame& frame) {
			RenderLines(camera);
		} });

		m_RenderPasses[RP_CameraCompositePass].push_back(RenderPass{ "Deferred Composite Pass", [this](CameraRef camera, const RenderFrame& frame) {
			DeferredCompositePass(camera, frame);
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
		GPUResourceManager* gpuResources = m_pEngine->GetMainModule<GraphicsModule>()->GetResourceManager();

		for (size_t i = 0; i < m_StaticPipelineRenderDatas.size(); ++i)
		{
			PipelineRenderData& pipelineRenderData = m_StaticPipelineRenderDatas[i];
			if (!pipelineRenderData.m_Dirty) continue;
			pipelineRenderData.m_IndirectDrawCommands.clear();
			pipelineRenderData.m_FinalPerObjectData.clear();
			pipelineRenderData.m_ObjectDataOffsets.clear();
			pipelineRenderData.m_pCombinedMesh->ClearIndices();
			pipelineRenderData.m_pCombinedMesh->ClearVertices();
			pipelineRenderData.m_UniqueMeshOrder.clear();
			pipelineRenderData.m_UniqueMeshOrder.reserve(pipelineRenderData.m_Meshes.size());

			for (auto& iter : pipelineRenderData.m_Meshes)
			{
				PipelineMeshRenderData& meshRenderData = iter.second;
				pipelineRenderData.m_UniqueMeshOrder.emplace_back(meshRenderData.m_Mesh);
				Resource* pMeshResource = m_pEngine->GetAssetManager().FindResource(meshRenderData.m_Mesh);
				MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
				const int baseVertex = (int)pipelineRenderData.m_pCombinedMesh->VertexCount();
				const uint32_t firstIndex = (int)pipelineRenderData.m_pCombinedMesh->IndexCount();

				pipelineRenderData.m_ObjectDataOffsets.emplace_back(pipelineRenderData.m_FinalPerObjectData->size());
				pipelineRenderData.m_pCombinedMesh->Merge(pMeshData);
				const size_t objectDataStart = pipelineRenderData.m_FinalPerObjectData->size();

				DrawElementsIndirectCommand command;
				command.BaseInstance = 0;
				command.InstanceCount = meshRenderData.m_Objects.size();
				command.Count = pMeshData->IndexCount();
				command.BaseVertex = baseVertex;
				command.FirstIndex = firstIndex;
				pipelineRenderData.m_IndirectDrawCommands.emplace_back(std::move(command));

				for (size_t i = 0; i < meshRenderData.m_Materials.size(); ++i)
				{
					const UUID materialID = meshRenderData.m_Materials[i];
					auto materialIter = std::find(pipelineRenderData.m_UniqueMaterials->begin(), pipelineRenderData.m_UniqueMaterials->end(), materialID);
					const bool hasMaterial = materialIter != pipelineRenderData.m_UniqueMaterials->end();
					const size_t index = hasMaterial ? materialIter - pipelineRenderData.m_UniqueMaterials->begin() : pipelineRenderData.m_UniqueMaterials->size();
					if (!hasMaterial) pipelineRenderData.m_UniqueMaterials.emplace_back(materialID);
					meshRenderData.m_Objects[i].m_MaterialIndex = (uint32_t)index;
				}
				pipelineRenderData.m_FinalPerObjectData.resize(objectDataStart + meshRenderData.m_Objects.size());
				std::memcpy(&pipelineRenderData.m_FinalPerObjectData.m_Data[objectDataStart], meshRenderData.m_Objects.data(),
					sizeof(PerObjectData)*meshRenderData.m_Objects.size());
			}
			pipelineRenderData.m_Dirty = false;
		}

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
			pass.m_Callback(nullptr, m_FrameData);
		}

		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
			pRenderTexture->BindForDraw();
			pGraphics->Clear(camera.GetClearColor());

			for (auto& pass : m_RenderPasses[RP_CameraPrepass])
			{
				pass.m_Callback(camera, m_FrameData);
			}

			OnStartCameraRender(camera, m_FrameData.ActiveLights);

			for (auto& pass : m_RenderPasses[RP_ObjectPass])
			{
				pass.m_Callback(camera, m_FrameData);
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
				pass.m_Callback(camera, m_FrameData);
			}

			OnEndCameraRender(camera, m_FrameData.ActiveLights);
			pRenderTexture->UnBindForDraw();
			OnRenderEffects(camera, pRenderTexture);

			for (auto& pass : m_RenderPasses[RP_CameraPostpass])
			{
				pass.m_Callback(camera, m_FrameData);
			}
		}

		for (auto& pass : m_RenderPasses[RP_PreCompositePass])
		{
			pass.m_Callback(nullptr, m_FrameData);
		}

		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
			pRenderTexture->BindForDraw();
			for (auto& pass : m_RenderPasses[RP_CameraCompositePass])
			{
				pass.m_Callback(camera, m_FrameData);
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
			pass.m_Callback(nullptr, m_FrameData);
		}

		for (auto& pass : m_RenderPasses[RP_Postpass])
		{
			pass.m_Callback(nullptr, m_FrameData);
		}

		m_LastSubmittedObjectCount = m_FrameData.ObjectsToRender.size();
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
		m_pLineBuffer = pGraphics->GetResourceManager()->CreateBuffer(sizeof(LineVertex) * MAX_LINE_VERTICES, BufferBindingTarget::B_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
		m_pLineMesh = pGraphics->GetResourceManager()->CreateMesh(MAX_LINE_VERTICES, 0, InputRate::Vertex, 0, sizeof(LineVertex), PrimitiveType::PT_Lines, { AttributeType::Float3, AttributeType::Float4 }, m_pLineBuffer, nullptr);
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

	void RendererModule::MainObjectPass(CameraRef camera, const RenderFrame& frame)
	{
		/* Render objects */
		for (size_t j = 0; j < m_FrameData.ObjectsToRender.size(); ++j)
		{
			LayerMask mask = camera.GetLayerMask();
			if (mask != 0 && (mask & m_FrameData.ObjectsToRender[j].m_LayerMask) == 0) continue;
			m_pEngine->Profiler().BeginSample("RendererModule::OnRender");
			OnRender(camera, m_FrameData.ObjectsToRender[j]);
			m_pEngine->Profiler().EndSample();
		}
	}

	void RendererModule::SkyboxPass(CameraRef camera, const RenderFrame&)
	{
		GScene* pActiveScene = m_pEngine->GetSceneManager()->GetActiveScene();
		if (!pActiveScene) return;
		const UUID skyboxID = pActiveScene->Settings().m_LightingSettings.m_Skybox;
		if (!skyboxID) return;
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(skyboxID);
		if (!pResource) return;
		CubemapData* pCubemap = static_cast<CubemapData*>(pResource);
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		pGraphics->EnableDepthWrite(false);
		OnRenderSkybox(camera, pCubemap);
		pGraphics->EnableDepthWrite(true);
	}

	void RendererModule::MainTextPass(CameraRef camera, const RenderFrame& frame)
	{
		/* Render texts */
		for (size_t j = 0; j < m_FrameData.TextsToRender.size(); ++j)
		{
			LayerMask mask = camera.GetLayerMask();
			if (mask != 0 && (mask & m_FrameData.TextsToRender[j].m_LayerMask) == 0) continue;
			m_pEngine->Profiler().BeginSample("RendererModule::OnRender rendering text object");
			OnRender(camera, m_FrameData.TextsToRender[j]);
			m_pEngine->Profiler().EndSample();
		}
	}

	void RendererModule::MainLateObjectPass(CameraRef camera, const RenderFrame& frame)
	{
		for (size_t j = 0; j < m_FrameData.ObjectsToRenderLate.size(); ++j)
		{
			LayerMask mask = camera.GetLayerMask();
			if (mask != 0 && (mask & m_FrameData.ObjectsToRenderLate[j].m_LayerMask) == 0) continue;
			m_pEngine->Profiler().BeginSample("RendererModule::OnRender with late render object");
			OnRender(camera, m_FrameData.ObjectsToRenderLate[j]);
			m_pEngine->Profiler().EndSample();
		}
	}

	void RendererModule::DeferredCompositePass(CameraRef camera, const RenderFrame& frame)
	{
		/* Composite to cameras render texture */
		RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
		RenderTexture* pOutputTexture = camera.GetOutputTexture();
		RenderTexture* pSecondaryOutputTexture = camera.GetSecondaryOutputTexture();
		const glm::uvec2& resolution = camera.GetResolution();
		if (pOutputTexture == nullptr)
		{
			pOutputTexture = m_pEngine->GetDisplayManager().CreateOutputTexture(m_pEngine, resolution.x, resolution.y);
			camera.SetOutputTexture(pOutputTexture);
		}
		if (pSecondaryOutputTexture == nullptr)
		{
			pSecondaryOutputTexture = m_pEngine->GetDisplayManager().CreateOutputTexture(m_pEngine, resolution.x, resolution.y);
			camera.SetSecondaryOutputTexture(pSecondaryOutputTexture);
		}
		uint32_t width, height;
		pOutputTexture->GetDimensions(width, height);
		if (width != resolution.x || height != resolution.y)
		{
			pOutputTexture->Resize(resolution.x, resolution.y);
			pSecondaryOutputTexture->Resize(resolution.x, resolution.y);
			pOutputTexture->GetDimensions(width, height);
		}

		m_pEngine->Profiler().BeginSample("RendererModule::OnRender > Output Rendering");
		pOutputTexture->BindForDraw();
		OnDoCompositing(camera, width, height, pRenderTexture);
		pOutputTexture->UnBindForDraw();
		m_pEngine->Profiler().EndSample();
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
		Render();
	}

	void RendererModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterAssetReference<PipelineData>("Lines Pipeline", 19);
	}

	PipelineRenderData::PipelineRenderData(UUID pipeline) : m_Pipeline(pipeline),
		m_pCombinedMesh(new MeshData(1000000, sizeof(DefaultVertex3D),
			{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
			AttributeType::Float3 , AttributeType::Float2, AttributeType::Float4 })),
		m_Dirty(false), m_pIndirectDrawCommandsBuffer(nullptr),
		m_pIndirectDrawPerObjectDataBuffer(nullptr), m_pIndirectObjectDataOffsetsBuffer(nullptr),
		m_pIndirectMaterialPropertyData(nullptr)
	{
	}

	PipelineRenderData::~PipelineRenderData()
	{
		delete m_pCombinedMesh;
		m_pCombinedMesh = nullptr;
	}
}
