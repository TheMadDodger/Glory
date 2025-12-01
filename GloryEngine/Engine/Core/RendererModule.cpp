#include "RendererModule.h"
#include "Engine.h"
#include "Console.h"
#include "EngineProfiler.h"
#include "WindowModule.h"
#include "MaterialData.h"
#include "PipelineData.h"
#include "CameraManager.h"
#include "AssetManager.h"
#include "GPUTextureAtlas.h"
#include "GraphicsEnums.h"

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

namespace Glory
{
	RendererModule::RendererModule()
		: m_LastSubmittedObjectCount(0), m_LastSubmittedCameraCount(0), m_LineVertexCount(0),
		m_pLineVertex(nullptr), m_pLineVertices(nullptr),
		m_FrameData(size_t(MAX_LIGHTS))
	{
	}

	RendererModule::~RendererModule()
	{
	}

	const std::type_info& RendererModule::GetModuleType()
	{
		return typeid(RendererModule);
	}

	void RendererModule::SubmitStatic(RenderData&& renderData)
	{
		ProfileSample sample{ &m_pEngine->Profiler(), "RendererModule::SubmitStatic" };

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
		meshIter->second.m_LayerMasks.emplace_back(renderData.m_LayerMask);
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
		ProfileSample sample{ &m_pEngine->Profiler(), "RendererModule::UpdateStatic" };

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
		ProfileSample sample{ &m_pEngine->Profiler(), "RendererModule::UnsubmitStatic" };

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
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::SubmitDynamic" };

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
		meshIter->second.m_LayerMasks.emplace_back(renderData.m_LayerMask);
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
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::SubmitLate" };

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
		meshIter->second.m_LayerMasks.emplace_back(renderData.m_LayerMask);
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

	void RendererModule::SubmitCamera(CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::SubmitCamera" };
		auto it = std::find_if(m_ActiveCameras.begin(), m_ActiveCameras.end(), [camera, this](const CameraRef& other)
		{
			return camera.GetPriority() < other.GetPriority();
		});

		if (it != m_ActiveCameras.end())
			m_ActiveCameras.insert(it, camera);
		else
			m_ActiveCameras.push_back(camera);

		if (camera.IsOutput())
		{
			auto it = std::find_if(m_OutputCameras.begin(), m_OutputCameras.end(), [camera, this](const CameraRef& other)
			{
				return camera.GetPriority() < other.GetPriority();
			});

			if (it != m_OutputCameras.end())
				m_OutputCameras.insert(it, camera);
			else
				m_OutputCameras.push_back(camera);
		}
		OnSubmitCamera(camera);
	}

	void RendererModule::UnsubmitCamera(CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::UnsubmitCamera" };

		auto iter = std::find(m_ActiveCameras.begin(), m_ActiveCameras.end(), camera);
		if (iter == m_ActiveCameras.end()) return;
		m_ActiveCameras.erase(iter);

		if (camera.IsOutput())
		{
			auto outputIter = std::find(m_OutputCameras.begin(), m_OutputCameras.end(), camera);
			if (outputIter != m_OutputCameras.end()) return;
			m_OutputCameras.erase(outputIter);
		}

		OnUnsubmitCamera(camera);
	}

	void RendererModule::UpdateCamera(CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::UpdateCamera" };
		auto iter = std::find(m_ActiveCameras.begin(), m_ActiveCameras.end(), camera);
		auto outputIter = std::find(m_OutputCameras.begin(), m_OutputCameras.end(), camera);
		if (iter == m_ActiveCameras.end()) return;

		static auto comparer = [](const CameraRef& a, const CameraRef& b) {
			return a.GetPriority() < b.GetPriority();
		};

		std::sort(m_ActiveCameras.begin(), m_ActiveCameras.end(), comparer);
		std::sort(m_OutputCameras.begin(), m_OutputCameras.end(), comparer);

		if (camera.IsOutput() && outputIter == m_OutputCameras.end())
		{
			auto it = std::find_if(m_OutputCameras.begin(), m_OutputCameras.end(), [camera, this](const CameraRef& other)
			{
				return camera.GetPriority() < other.GetPriority();
			});

			if (it != m_OutputCameras.end())
				m_OutputCameras.insert(it, camera);
			else
				m_OutputCameras.push_back(camera);
		}
		else if (!camera.IsOutput() && outputIter != m_OutputCameras.end())
			m_OutputCameras.erase(outputIter);

		if (camera.IsResolutionDirty())
			OnCameraResize(camera);
		if (camera.IsPerspectiveDirty())
			OnCameraPerspectiveChanged(camera);

		OnCameraUpdated(camera);
	}

	size_t RendererModule::Submit(const glm::ivec2& pickPos, UUID cameraID)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(pickPos, cameraID)" };
		const size_t index = m_FrameData.Picking.size();
		m_FrameData.Picking.push_back({ pickPos, cameraID });
		return index;
	}

	void RendererModule::Submit(LightData&& light, glm::mat4&& lightView, glm::mat4&& lightProjection, UUID id)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Submit(light)" };
		const size_t index = m_FrameData.ActiveLights.count();
		m_FrameData.ActiveLights.push_back(std::move(light));
		m_FrameData.LightViews.push_back(std::move(lightView));
		m_FrameData.LightProjections.push_back(std::move(lightProjection));
		m_FrameData.ActiveLightIDs.push_back(id);
		OnSubmit(m_FrameData.ActiveLights[index]);
	}

	void RendererModule::OnBeginFrame()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::OnBeginFrame" };
		//REQUIRE_MODULE(m_pEngine, GraphicsModule, );
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		m_FrameData.Reset();
		std::for_each(m_DynamicPipelineRenderDatas.begin(), m_DynamicPipelineRenderDatas.end(), [](PipelineBatch& batch) { batch.Reset(); });
		std::for_each(m_DynamicLatePipelineRenderDatas.begin(), m_DynamicLatePipelineRenderDatas.end(), [](PipelineBatch& batch) { batch.Reset(); });
	}

	void RendererModule::OnEndFrame()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::OnEndFrame" };
		m_LastResolution = m_Resolution;

		m_pLineVertex = m_pLineVertices;
		m_LineVertexCount = 0;
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
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::DrawLine" };
		const glm::vec4 transfromedP1 = transform * glm::vec4(p1, 1.0f);
		const glm::vec4 transfromedP2 = transform * glm::vec4(p2, 1.0f);

		if (m_LineVertexCount + 1 >= MAX_LINE_VERTICES)
		{
			m_pEngine->GetDebug().LogError("RendererModule::DrawLine: Exceeded max line vertixes.");
			return;
		}

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
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::DrawLineQuad" };
		DrawLine(transform, p1, p2, color);
		DrawLine(transform, p2, p3, color);
		DrawLine(transform, p3, p4, color);
		DrawLine(transform, p4, p1, color);
	}

	void RendererModule::DrawLineCircle(const glm::mat4& transform, const glm::vec3& position, float radius, CircleUp up, const glm::vec4& color)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::DrawLineCircle" };
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
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::DrawLineBox" };
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
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::DrawLineSphere" };
		DrawLineCircle(transform, position, radius, CircleUp::x, color);
		DrawLineCircle(transform, position, radius, CircleUp::y, color);
		DrawLineCircle(transform, position, radius, CircleUp::z, color);
	}

	void RendererModule::DrawLineShape(const glm::mat4& transform, const glm::vec3& position, const ShapeProperty& shape, const glm::vec4& color)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::DrawLineShape" };
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
		return m_PickResults.size() > index;
	}

	bool RendererModule::PickResultIndex(UUID cameraID, size_t& index) const
	{
		auto iter = std::find_if(m_PickResults.begin(), m_PickResults.end(), [cameraID](const PickResult& result) {
			return result.m_CameraID == cameraID;
		});
		if (iter == m_PickResults.end()) return false;
		index = iter - m_PickResults.begin();
		return true;
	}

	const PickResult& RendererModule::GetPickResult(size_t index) const
	{
		return m_PickResults[index];
	}

	void RendererModule::GetPickResult(UUID cameraID, std::function<void(const PickResult&)> callback)
	{
		std::scoped_lock<std::mutex> lock(m_PickLock);
		size_t index;
		if (!PickResultIndex(cameraID, index)) return;
		callback(m_PickResults[index]);
	}

	void RendererModule::OnWindowResize(glm::uvec2 size)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::OnWindowResize" };
		if (size == m_Resolution || size.x == 0 || size.y == 0) return;
		m_Resolution = size;
		OnWindowResized();
	}

	size_t RendererModule::CreateGPUTextureAtlas(TextureCreateInfo&& textureInfo, TextureHandle texture)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::CreateGPUTextureAtlas" };
		const size_t index = m_GPUTextureAtlases.size();
		GPUTextureAtlas& newAtlas = m_GPUTextureAtlases.emplace_back(std::move(textureInfo), m_pEngine, texture);
		newAtlas.Initialize();
		return index;
	}

	GPUTextureAtlas& RendererModule::GetGPUTextureAtlas(size_t index)
	{
		return m_GPUTextureAtlases[index];
	}

	const GPUTextureAtlas& RendererModule::GetGPUTextureAtlas(size_t index) const
	{
		return m_GPUTextureAtlases[index];
	}

	void RendererModule::Reset()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Reset" };
		m_StaticPipelineRenderDatas.clear();
		m_DynamicPipelineRenderDatas.clear();
		m_DynamicLatePipelineRenderDatas.clear();
		m_FrameData.Reset();
	}

	CameraRef RendererModule::GetActiveCamera(uint32_t cameraIndex) const
	{
		return m_ActiveCameras[cameraIndex];
	}

	CameraRef RendererModule::GetOutputCamera(uint32_t cameraIndex) const
	{
		return m_OutputCameras[cameraIndex];
	}

	size_t RendererModule::GetOutputCameraCount() const
	{
		return m_OutputCameras.size();
	}

	void RendererModule::Initialize()
	{
		m_pLineVertices = new LineVertex[MAX_LINE_VERTICES];
		m_pLineVertex = m_pLineVertices;
	}

	void RendererModule::PostInitialize()
	{
		OnPostInitialize();
	}

	void RendererModule::OnCameraResize(CameraRef camera)
	{
		camera.SetResolutionDirty(false);
	}

	void RendererModule::OnCameraPerspectiveChanged(CameraRef camera)
	{
		camera.SetPerspectiveDirty(false);
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

	bool RendererModule::ResolutionChanged() const
	{
		return m_LastResolution != m_Resolution;
	}

	const glm::uvec2& RendererModule::Resolution() const
	{
		return m_Resolution;
	}

	void RendererModule::SetSwapchain(SwapchainHandle swapchain)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::SetSwapchain" };
		assert(m_Swapchain == nullptr);
		m_Swapchain = swapchain;
		OnSwapchainChanged();
	}

	void RendererModule::SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
	}

	void RendererModule::InjectSwapchainSubpass(std::function<void(GraphicsDevice*, RenderPassHandle, CommandBufferHandle)> subpassFunc)
	{
		m_InjectedSwapchainSubpasses.push_back(subpassFunc);
	}

	void RendererModule::AddPostProcess(PostProcess&& postProcess)
	{
		m_PostProcesses.emplace_back(std::move(postProcess));
		static auto comparer = [](const PostProcess& a, const PostProcess& b) {
			return a.m_Priority > b.m_Priority;
		};
		std::sort(m_PostProcesses.begin(), m_PostProcesses.end(), comparer);
	}
}
