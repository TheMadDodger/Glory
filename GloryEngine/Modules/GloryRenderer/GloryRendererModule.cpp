#include "GloryRendererModule.h"

#include <Engine.h>
#include <GraphicsDevice.h>

#include <PipelineManager.h>
#include <MaterialManager.h>
#include <AssetManager.h>

#include <PipelineData.h>
#include <MeshData.h>

#include <EngineProfiler.h>

namespace Glory
{
	static uint32_t* ResetLightDistances;

	constexpr size_t AttachmentNameCount = 6;
	constexpr std::string_view AttachmentNames[] = {
		"object",
		"Debug",
		"Color",
		"Normal",
		"AOBlurred",
		"Data",
	};

	GLORY_MODULE_VERSION_CPP(GloryRendererModule);

	enum class BindingIndices : uint32_t
	{
		RenderConstants = 1,
		CameraDatas = 2,
		WorldTransforms = 3,
		Materials = 4,
		HasTexture = 6,
	};

	GloryRendererModule::GloryRendererModule()
	{
	}

	GloryRendererModule::~GloryRendererModule()
	{
	}

	void GloryRendererModule::OnCameraResize(CameraRef camera)
	{
		/* When the camera rendertexture resizes we need to generate a new grid of clusters for that camera */
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		DescriptorSetHandle clusterSet = camera.GetUserHandle("ClusterSet");
		if (!clusterSet) return; // Should not happen but just in case
		GenerateClusterSSBO(pDevice, camera, clusterSet);
	}

	void GloryRendererModule::OnCameraPerspectiveChanged(CameraRef camera)
	{
		/* When the camera changed perspective we need to generate a new grid of clusters for that camera */
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		DescriptorSetHandle clusterSet = camera.GetUserHandle("ClusterSet");
		if (!clusterSet) return; // Should not happen but just in case
		GenerateClusterSSBO(pDevice, camera, clusterSet);
	}

	MaterialData* GloryRendererModule::GetInternalMaterial(std::string_view name) const
	{
		return nullptr;
	}

	void GloryRendererModule::CollectReferences(std::vector<UUID>& references)
	{
		ModuleSettings& settings = Settings();

		std::vector<UUID> newReferences;
		newReferences.push_back(settings.Value<uint64_t>("Lines Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Screen Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("SSAO Prepass Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("SSAO Blur Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Text Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Display Copy Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Skybox Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Shadows Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Shadows Transparent Textured Pipeline"));

		newReferences.push_back(settings.Value<uint64_t>("Cluster Generator"));
		newReferences.push_back(settings.Value<uint64_t>("Cluster Cull Light"));

		for (size_t i = 0; i < newReferences.size(); ++i)
		{
			if (!newReferences[i]) continue;
			references.push_back(newReferences[i]);
			Resource* pPipelineResource = m_pEngine->GetAssetManager().GetAssetImmediate(newReferences[i]);
			if (!pPipelineResource) continue;
			PipelineData* pPipelineData = static_cast<PipelineData*>(pPipelineResource);
			for (size_t i = 0; i < pPipelineData->ShaderCount(); ++i)
			{
				const UUID shaderID = pPipelineData->ShaderID(i);
				if (!shaderID) continue;
				references.push_back(shaderID);
			}
		}
	}

	UUID GloryRendererModule::TextPipelineID() const
	{
		const ModuleSettings& settings = Settings();
		return settings.Value<uint64_t>("Text Pipeline");
	}

	void GloryRendererModule::Initialize()
	{
		RendererModule::Initialize();
	}

	void GloryRendererModule::OnPostInitialize()
	{
		const ModuleSettings& settings = Settings();
		const UUID screenPipeline = settings.Value<uint64_t>("Screen Pipeline");
		const UUID SSAOPrePassPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
		const UUID SSAOBlurPipeline = settings.Value<uint64_t>("SSAO Blur Pipeline");
		const UUID textPipeline = settings.Value<uint64_t>("Text Pipeline");
		const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");
		const UUID skyboxPipeline = settings.Value<uint64_t>("Skybox Pipeline");
		const UUID irradiancePipeline = settings.Value<uint64_t>("Irradiance Pipeline");
		const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
		const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");
		const UUID clusterGeneratorPipeline = settings.Value<uint64_t>("Cluster Generator");
		const UUID clusterCullLightPipeline = settings.Value<uint64_t>("Cluster Cull Light");

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice)
		{
			m_pEngine->GetDebug().LogError("Renderer: No graphics device active");
			return;
		}

		const bool usePushConstants = pDevice->IsSupported(APIFeatures::PushConstants);
		DescriptorSetLayoutInfo setLayoutInfo;
		DescriptorSetInfo setInfo;
		setLayoutInfo.m_Buffers.resize(usePushConstants ? 1 : 2);
		setInfo.m_Buffers.resize(usePushConstants ? 1 : 2);
		if (!usePushConstants)
		{
			m_RenderConstantsBuffer = setInfo.m_Buffers[0].m_BufferHandle = pDevice->CreateBuffer(sizeof(RenderConstants), BufferType::BT_Uniform);
			setInfo.m_Buffers[0].m_Offset = 0;
			setInfo.m_Buffers[0].m_Size = sizeof(RenderConstants);
			setLayoutInfo.m_Buffers[0].m_BindingIndex = uint32_t(BindingIndices::RenderConstants);
			setLayoutInfo.m_Buffers[0].m_Type = BufferType::BT_Uniform;
			setLayoutInfo.m_Buffers[0].m_ShaderStages = ShaderTypeFlag(STF_Vertex | STF_Fragment);
		}
		else
		{
			setLayoutInfo.m_PushConstantRange.m_Offset = 0;
			setLayoutInfo.m_PushConstantRange.m_Size = sizeof(RenderConstants);
			setLayoutInfo.m_PushConstantRange.m_ShaderStages = ShaderTypeFlag(STF_Vertex | STF_Fragment);
		}
		const size_t cameraDatasBufferIndex = usePushConstants ? 0 : 1;
		m_CameraDatasBuffer = setInfo.m_Buffers[cameraDatasBufferIndex].m_BufferHandle = pDevice->CreateBuffer(sizeof(PerCameraData)*MAX_CAMERAS, BufferType::BT_Uniform);
		setLayoutInfo.m_Buffers[cameraDatasBufferIndex].m_BindingIndex = uint32_t(BindingIndices::CameraDatas);
		setLayoutInfo.m_Buffers[cameraDatasBufferIndex].m_Type = BufferType::BT_Uniform;
		setLayoutInfo.m_Buffers[cameraDatasBufferIndex].m_ShaderStages = ShaderTypeFlag(STF_Vertex);
		setInfo.m_Buffers[cameraDatasBufferIndex].m_Offset = 0;
		setInfo.m_Buffers[cameraDatasBufferIndex].m_Size = sizeof(PerCameraData)*MAX_CAMERAS;
		//m_LightCameraDatasBuffer = pDevice->CreateBuffer(CameraDatasBufferName, sizeof(PerCameraData)*MAX_LIGHTS, BufferType::BT_Uniform);

		m_GlobalSetLayout = setInfo.m_Layout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
		m_GlobalSet = pDevice->CreateDescriptorSet(std::move(setInfo));

		setLayoutInfo = DescriptorSetLayoutInfo();
		setLayoutInfo.m_Buffers.resize(2);
		setLayoutInfo.m_Buffers[0].m_BindingIndex = 1;
		setLayoutInfo.m_Buffers[0].m_Type = BufferType::BT_Storage;
		setLayoutInfo.m_Buffers[0].m_ShaderStages = STF_Compute;
		setLayoutInfo.m_Buffers[1].m_BindingIndex = 2;
		setLayoutInfo.m_Buffers[1].m_Type = BufferType::BT_Storage;
		setLayoutInfo.m_Buffers[1].m_ShaderStages = STF_Compute;
		m_ClusterSetLayout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));

		setLayoutInfo = DescriptorSetLayoutInfo();
		setLayoutInfo.m_Buffers.resize(7);
		for (size_t i = 0; i < setLayoutInfo.m_Buffers.size(); ++i)
		{
			setLayoutInfo.m_Buffers[i].m_BindingIndex = static_cast<uint32_t>(i + 1);
			setLayoutInfo.m_Buffers[i].m_Type = BufferType::BT_Storage;
			setLayoutInfo.m_Buffers[i].m_ShaderStages = STF_Compute;
		}
		m_ClusterCullLightSetLayout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));

		m_ScreenToViewBuffer = pDevice->CreateBuffer(sizeof(ScreenToView), BufferType::BT_Storage);
		pDevice->AssignBuffer(m_ScreenToViewBuffer, &ScreenToView(), sizeof(ScreenToView));

		m_LightsSSBO = pDevice->CreateBuffer(sizeof(LightData)*MAX_LIGHTS, BufferType::BT_Storage);
		m_LightCountSSBO = pDevice->CreateBuffer(sizeof(uint32_t), BufferType::BT_Storage);
		//m_LightSpaceTransformsSSBO = pDevice->CreateBuffer(sizeof(glm::mat4)*MAX_LIGHTS, BufferType::BT_Storage);
		m_LightDistancesSSBO = pDevice->CreateBuffer(sizeof(uint32_t)*MAX_LIGHTS, BufferType::BT_Storage);

		ResetLightDistances = new uint32_t[MAX_LIGHTS];
		for (size_t i = 0; i < MAX_LIGHTS; ++i)
			ResetLightDistances[i] = NUM_DEPTH_SLICES;
		pDevice->AssignBuffer(m_LightDistancesSSBO, ResetLightDistances);
	}

	void GloryRendererModule::Update()
	{
		ModuleSettings& settings = Settings();
		if (!settings.IsDirty()) return;

		const UUID screenPipeline = settings.Value<uint64_t>("Screen Pipeline");
		const UUID SSAOPrePassPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
		const UUID SSAOBlurPipeline = settings.Value<uint64_t>("SSAO Blur Pipeline");
		const UUID textPipeline = settings.Value<uint64_t>("Text Pipeline");
		const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");
		const UUID skyboxPipeline = settings.Value<uint64_t>("Skybox Pipeline");
		const UUID irradiancePipeline = settings.Value<uint64_t>("Irradiance Pipeline");
		const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
		const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");
		const UUID clusterGeneratorPipeline = settings.Value<uint64_t>("Cluster Generator");
		const UUID clusterCullLightPipeline = settings.Value<uint64_t>("Cluster Cull Light");

		settings.SetDirty(false);
	}

	void GloryRendererModule::Draw()
	{
		//m_pEngine->GetDebug().SubmitLines(this, &m_pEngine->Time());
		//Render();

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		/* Make sure every camera has a render pass */
		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			RenderPassHandle& renderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("RenderPass"));
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

		m_PickResults.clear();

		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Render" };

		PrepareDataPass();

		m_CommandBuffer = pDevice->Begin();

		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			RenderPassHandle& renderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("RenderPass"));
			ClusterPass(static_cast<uint32_t>(i));
			pDevice->BeginRenderPass(m_CommandBuffer, renderPass);
			DynamicObjectsPass(static_cast<uint32_t>(i));
			pDevice->EndRenderPass(m_CommandBuffer);
		}

		pDevice->End(m_CommandBuffer);
		pDevice->Commit(m_CommandBuffer);
		pDevice->Wait(m_CommandBuffer);
		pDevice->Release(m_CommandBuffer);

		//m_LastSubmittedObjectCount = m_FrameData.ObjectsToRender.size();
		//m_LastSubmittedCameraCount = m_FrameData.ActiveCameras.size();

		//std::scoped_lock lock(m_PickLock);
		//m_LastFramePickResults.resize(m_PickResults.size());
		//std::memcpy(m_LastFramePickResults.data(), m_PickResults.data(), m_PickResults.size()*sizeof(PickResult));
	}

	void GloryRendererModule::Cleanup()
	{
	}

	void GloryRendererModule::LoadSettings(ModuleSettings& settings)
	{
		RendererModule::LoadSettings(settings);
		settings.RegisterAssetReference<PipelineData>("Screen Pipeline", 20);
		settings.RegisterAssetReference<PipelineData>("SSAO Prepass Pipeline", 21);
		settings.RegisterAssetReference<PipelineData>("SSAO Blur Pipeline", 22);
		settings.RegisterAssetReference<PipelineData>("Text Pipeline", 23);
		settings.RegisterAssetReference<PipelineData>("Display Copy Pipeline", 30);
		settings.RegisterAssetReference<PipelineData>("Skybox Pipeline", 33);
		settings.RegisterAssetReference<PipelineData>("Irradiance Pipeline", 35);
		settings.RegisterAssetReference<PipelineData>("Shadows Pipeline", 38);
		settings.RegisterAssetReference<PipelineData>("Shadows Transparent Textured Pipeline", 39);
		settings.RegisterAssetReference<PipelineData>("Cluster Generator", 44);
		settings.RegisterAssetReference<PipelineData>("Cluster Cull Light", 45);
	}

	size_t GloryRendererModule::CameraAttachmentPreviewCount() const
	{
		return AttachmentNameCount;
	}

	std::string_view GloryRendererModule::CameraAttachmentPreviewName(size_t index) const
	{
		return AttachmentNames[index];
	}

	TextureHandle GloryRendererModule::CameraAttachmentPreview(CameraRef camera, size_t index) const
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		RenderPassHandle renderPass = camera.GetUserHandle("RenderPass");
		if (!renderPass) return NULL;
		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
		return pDevice->GetRenderTextureAttachment(renderTexture, index);
	}

	size_t GloryRendererModule::GetGCD(size_t a, size_t b)
	{
		if (b == 0)
			return a;
		return GetGCD(b, a % b);
	}

	float lerp(float a, float b, float f)
	{
		return a + f*(b - a);
	}

	void GloryRendererModule::RenderBatches(const std::vector<PipelineBatch>& batches, const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		MaterialManager& materialManager = m_pEngine->GetMaterialManager();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		AssetManager& assets = m_pEngine->GetAssetManager();

		const bool usePushConstants = pDevice->IsSupported(APIFeatures::PushConstants);

		RenderConstants constants;
		constants.m_CameraIndex = static_cast<uint32_t>(cameraIndex);
		if (!usePushConstants) pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));

		size_t batchIndex = 0;
		for (const PipelineBatch& pipelineRenderData : batches)
		{
			if (batchIndex >= batchDatas.size()) break;
			const PipelineBatchData& batchData = batchDatas.at(batchIndex);
			++batchIndex;

			PipelineData* pPipelineData = pipelines.GetPipelineData(pipelineRenderData.m_PipelineID);
			if (!pPipelineData) continue;
			pDevice->BeginPipeline(m_CommandBuffer, batchData.m_Pipeline);
			pDevice->BindDescriptorSets(m_CommandBuffer, batchData.m_Pipeline, { m_GlobalSet, batchData.m_Set });

			uint32_t objectIndex = 0;
			for (UUID uniqueMeshID : pipelineRenderData.m_UniqueMeshOrder)
			{
				const PipelineMeshBatch& meshBatch = pipelineRenderData.m_Meshes.at(uniqueMeshID);
				Resource* pMeshResource = assets.FindResource(meshBatch.m_Mesh);
				if (!pMeshResource) continue;
				MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
				MeshHandle mesh = pDevice->AcquireCachedMesh(pMeshData);
				if (!mesh) continue;

				for (size_t i = 0; i < meshBatch.m_Worlds.size(); ++i)
				{
					const uint32_t currentObject = objectIndex;
					++objectIndex;

					const UUID materialID = pipelineRenderData.m_UniqueMaterials[meshBatch.m_MaterialIndices[i]];
					MaterialData* pMaterialData = materialManager.GetMaterial(materialID);
					if (!pMaterialData) continue;

					const auto& ids = meshBatch.m_ObjectIDs[i];
					constants.m_ObjectID = ids.second;
					constants.m_SceneID = ids.first;
					constants.m_ObjectDataIndex = currentObject;
					constants.m_MaterialIndex = meshBatch.m_MaterialIndices[i];

					if (usePushConstants)
						pDevice->PushConstants(m_CommandBuffer, batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants);
					else
						pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));
					if (!batchData.m_TextureSets.empty())
						pDevice->BindDescriptorSets(m_CommandBuffer, batchData.m_Pipeline, { batchData.m_TextureSets[constants.m_MaterialIndex] }, 2);
					pDevice->DrawMesh(m_CommandBuffer, mesh);
				}
			}

			pDevice->EndPipeline(m_CommandBuffer);
		}
	}

	void GloryRendererModule::PrepareDataPass()
	{
		const ModuleSettings& settings = Settings();
		const UUID clusterGeneratorPipeline = settings.Value<uint64_t>("Cluster Generator");
		const UUID clusterCullLightPipeline = settings.Value<uint64_t>("Cluster Cull Light");

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		if (!m_ClusterPipeline)
		{
			PipelineData* pClusterPipeline = pipelines.GetPipelineData(clusterGeneratorPipeline);
			m_ClusterPipeline = pDevice->CreateComputePipeline(pClusterPipeline, { m_ClusterSetLayout });
		}

		if (!m_ClusterCullLightPipeline)
		{
			PipelineData* pClusterCullLightPipeline = pipelines.GetPipelineData(clusterCullLightPipeline);
			m_ClusterCullLightPipeline = pDevice->CreateComputePipeline(pClusterCullLightPipeline, { m_ClusterCullLightSetLayout });
		}

		MaterialManager& materials = m_pEngine->GetMaterialManager();

		/* Prepare cameras */
		if (m_CameraDatas->size() < m_FrameData.ActiveCameras.size())
			m_CameraDatas.resize(m_FrameData.ActiveCameras.size());
		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_FrameData.ActiveCameras[i];

			const PerCameraData cameraData{ camera.GetView(), camera.GetProjection() };

			if (m_CameraDatas.m_Data[i].m_Projection != cameraData.m_Projection ||
				m_CameraDatas.m_Data[i].m_View != cameraData.m_View)
			{
				std::memcpy(&m_CameraDatas.m_Data[i], &cameraData, sizeof(PerCameraData));
				m_CameraDatas.m_Dirty = true;
			}

			BufferHandle& clusterSSBOHandle = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("ClusterSSBO"));
			if (!clusterSSBOHandle)
			{
				DescriptorSetHandle& clusterSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("ClusterSet"));
				DescriptorSetHandle& clusterCullSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("ClusterCullSet"));
				BufferHandle& lightIndexSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightIndexSSBO"));
				BufferHandle& lightGridSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightGridSSBO"));

				clusterSSBOHandle = pDevice->CreateBuffer(sizeof(VolumeTileAABB)*NUM_CLUSTERS, BufferType::BT_Storage);
				lightIndexSSBO = pDevice->CreateBuffer(sizeof(uint32_t)*(NUM_CLUSTERS*MAX_LIGHTS_PER_TILE + 1), BufferType::BT_Storage);
				lightGridSSBO = pDevice->CreateBuffer(sizeof(LightGrid)*NUM_CLUSTERS, BufferType::BT_Storage);

				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_ClusterSetLayout;
				setInfo.m_Buffers.resize(2);
				setInfo.m_Buffers[0].m_BufferHandle = clusterSSBOHandle;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(VolumeTileAABB)*NUM_CLUSTERS;
				setInfo.m_Buffers[1].m_BufferHandle = m_ScreenToViewBuffer;
				setInfo.m_Buffers[1].m_Offset = 0;
				setInfo.m_Buffers[1].m_Size = sizeof(ScreenToView);
				clusterSet = pDevice->CreateDescriptorSet(std::move(setInfo));

				setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_ClusterCullLightSetLayout;
				setInfo.m_Buffers.resize(7);
				setInfo.m_Buffers[0].m_BufferHandle = clusterSSBOHandle;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(VolumeTileAABB)*NUM_CLUSTERS;
				setInfo.m_Buffers[1].m_BufferHandle = m_ScreenToViewBuffer;
				setInfo.m_Buffers[1].m_Offset = 0;
				setInfo.m_Buffers[1].m_Size = sizeof(ScreenToView);
				setInfo.m_Buffers[2].m_BufferHandle = m_LightsSSBO;
				setInfo.m_Buffers[2].m_Offset = 0;
				setInfo.m_Buffers[2].m_Size = sizeof(LightData)*MAX_LIGHTS;
				setInfo.m_Buffers[3].m_BufferHandle = lightIndexSSBO;
				setInfo.m_Buffers[3].m_Offset = 0;
				setInfo.m_Buffers[3].m_Size = sizeof(uint32_t)*(NUM_CLUSTERS*MAX_LIGHTS_PER_TILE + 1);
				setInfo.m_Buffers[4].m_BufferHandle = lightGridSSBO;
				setInfo.m_Buffers[4].m_Offset = 0;
				setInfo.m_Buffers[4].m_Size = sizeof(LightGrid)*NUM_CLUSTERS;
				setInfo.m_Buffers[5].m_BufferHandle = m_LightDistancesSSBO;
				setInfo.m_Buffers[5].m_Offset = 0;
				setInfo.m_Buffers[5].m_Size = sizeof(uint32_t)*MAX_LIGHTS;
				setInfo.m_Buffers[6].m_BufferHandle = m_LightCountSSBO;
				setInfo.m_Buffers[6].m_Offset = 0;
				setInfo.m_Buffers[6].m_Size = sizeof(uint32_t);
				clusterCullSet = pDevice->CreateDescriptorSet(std::move(setInfo));

				GenerateClusterSSBO(pDevice, camera, clusterSet);
			}
		}
		if (m_CameraDatas)
			pDevice->AssignBuffer(m_CameraDatasBuffer, m_CameraDatas->data(),
				static_cast<uint32_t>(m_CameraDatas->size()*sizeof(PerCameraData)));

		/* Update light data */
		const uint32_t count = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		pDevice->AssignBuffer(m_LightCountSSBO, &count, sizeof(uint32_t));
		pDevice->AssignBuffer(m_LightsSSBO, m_FrameData.ActiveLights.data(), 0, MAX_LIGHTS*sizeof(LightData));
		//pDevice->AssignBuffer(m_LightSpaceTransformsSSBO, m_FrameData.LightSpaceTransforms.data(), 0, MAX_LIGHTS*sizeof(glm::mat4));
		//pDevice->AssignBuffer(m_LightDistancesSSBO, ResetLightDistances);

		/*if (m_LightCameraDatas->size() < m_FrameData.LightSpaceTransforms.count()) m_LightCameraDatas.resize(m_FrameData.LightSpaceTransforms.count());
		for (size_t i = 0; i < m_FrameData.LightSpaceTransforms.count(); ++i)
		{
			m_LightCameraDatas.m_Data[i].m_View = glm::identity<glm::mat4>();
			if (m_LightCameraDatas.m_Data[i].m_Projection != m_FrameData.LightSpaceTransforms[i])
			{
				m_LightCameraDatas.m_Data[i].m_Projection = m_FrameData.LightSpaceTransforms[i];
				m_LightCameraDatas.m_Dirty = true;
			}
		}*/
		//if (m_LightCameraDatas)
			//m_pLightCameraDatasBuffer->Assign(m_LightCameraDatas->data(), m_LightCameraDatas->size()*sizeof(PerCameraData));

		PrepareBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData);
	}

	void GloryRendererModule::PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas)
	{
		if (m_FrameData.ActiveCameras.empty()) return;
		CameraRef defaultCamera = m_FrameData.ActiveCameras[0];
		RenderPassHandle& defaultRenderPass = reinterpret_cast<RenderPassHandle&>(defaultCamera.GetUserHandle("RenderPass"));
		if (!defaultRenderPass) return;

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		MaterialManager& materials = m_pEngine->GetMaterialManager();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		AssetManager& assets = m_pEngine->GetAssetManager();

		/* Prepare dynamic data */
		size_t batchIndex = 0;
		batchDatas.reserve(batches.size());
		for (const auto& pipelineBatch : batches)
		{
			if (batchIndex >= batchDatas.size())
				batchDatas.emplace_back(PipelineBatchData{});

			PipelineData* pPipelineData = pipelines.GetPipelineData(pipelineBatch.m_PipelineID);
			if (!pPipelineData) continue;

			PipelineBatchData& batchData = batchDatas.at(batchIndex);
			++batchIndex;
			size_t meshIndex = 0;

			if (pipelineBatch.m_UniqueMeshOrder.empty()) continue;

			for (const UUID meshID : pipelineBatch.m_UniqueMeshOrder)
			{
				const PipelineMeshBatch& meshBatch = pipelineBatch.m_Meshes.at(meshID);
				if (batchData.m_Worlds->size() < meshIndex + meshBatch.m_Worlds.size())
					batchData.m_Worlds.resize(meshIndex + meshBatch.m_Worlds.size());

				if (std::memcmp(&batchData.m_Worlds.m_Data[meshIndex], meshBatch.m_Worlds.data(), meshBatch.m_Worlds.size()*sizeof(glm::mat4)) != 0)
				{
					std::memcpy(&batchData.m_Worlds.m_Data[meshIndex], meshBatch.m_Worlds.data(), meshBatch.m_Worlds.size()*sizeof(glm::mat4));
					batchData.m_Worlds.m_Dirty = true;
				}
				meshIndex += meshBatch.m_Worlds.size();
			}

			const size_t propertyDataSize = pPipelineData->TotalPropertiesByteSize();
			const size_t textureCount = pPipelineData->ResourcePropertyCount();
			const size_t paddingBytes = 16 - propertyDataSize % 16;
			const size_t finalPropertyDataSize = propertyDataSize + paddingBytes;
			const size_t totalBufferSize = finalPropertyDataSize*pipelineBatch.m_UniqueMaterials.size();
			if (batchData.m_MaterialDatas->size() < totalBufferSize)
				batchData.m_MaterialDatas.resize(totalBufferSize);
			if (textureCount && batchData.m_TextureBits->size() < pipelineBatch.m_UniqueMaterials.size())
				batchData.m_TextureBits.resize(pipelineBatch.m_UniqueMaterials.size());

			if (textureCount)
			{
				DescriptorSetLayoutInfo texturesSetLayoutInfo;
				texturesSetLayoutInfo.m_Samplers.resize(textureCount);
				texturesSetLayoutInfo.m_SamplerNames.resize(textureCount);
				for (size_t i = 0; i < texturesSetLayoutInfo.m_Samplers.size(); ++i)
				{
					texturesSetLayoutInfo.m_SamplerNames[i] = pPipelineData->ResourcePropertyInfo(i)->ShaderName();
					texturesSetLayoutInfo.m_Samplers[i].m_BindingIndex = static_cast<uint32_t>(i);
					texturesSetLayoutInfo.m_Samplers[i].m_ShaderStages = STF_Fragment;
				}
				batchData.m_TextureSetLayout = pDevice->CreateDescriptorSetLayout(std::move(texturesSetLayoutInfo));
				batchData.m_TextureSets.resize(pipelineBatch.m_UniqueMaterials.size(), nullptr);
			}

			for (size_t i = 0; i < pipelineBatch.m_UniqueMaterials.size(); ++i)
			{
				const UUID materialID = pipelineBatch.m_UniqueMaterials[i];
				MaterialData* pMaterialData = materials.GetMaterial(materialID);
				if (!pMaterialData) continue;
				const auto& buffer = pMaterialData->GetBufferReference();
				if (std::memcmp(&batchData.m_MaterialDatas.m_Data[i*finalPropertyDataSize], buffer.data(), buffer.size()) != 0)
				{
					std::memcpy(&batchData.m_MaterialDatas.m_Data[i*finalPropertyDataSize], buffer.data(), buffer.size());
					batchData.m_MaterialDatas.m_Dirty = true;
				}
				const uint32_t textureBits = pMaterialData->TextureSetBits();
				if (textureCount && batchData.m_TextureBits.m_Data[i] != textureBits)
				{
					batchData.m_TextureBits.m_Data[i] = textureBits;
					batchData.m_TextureBits.m_Dirty = true;
				}

				if (textureCount == 0) continue;

				/* Textures */
				if (!batchData.m_TextureSets[i])
				{
					DescriptorSetInfo setInfo;
					setInfo.m_Layout = batchData.m_TextureSetLayout;
					setInfo.m_Samplers.resize(textureCount);
					for (size_t j = 0; j < textureCount; ++j)
					{
						const UUID textureID = pMaterialData->GetResourceUUIDPointer(j)->AssetUUID();
						Resource* pResource = assets.FindResource(textureID);
						if (!pResource)
						{
							setInfo.m_Samplers[j].m_TextureHandle = 0;
							continue;
						}
						TextureData* pTexture = static_cast<TextureData*>(pResource);
						setInfo.m_Samplers[j].m_TextureHandle = pDevice->AcquireCachedTexture(pTexture);
					}
					batchData.m_TextureSets[i] = pDevice->CreateDescriptorSet(std::move(setInfo));
				}
			}

			if (!batchData.m_WorldsBuffer)
			{
				batchData.m_WorldsBuffer = pDevice->CreateBuffer(batchData.m_Worlds->size()*sizeof(glm::mat4), BT_Storage);
				batchData.m_Worlds.m_Dirty = true;
			}
			if (batchData.m_Worlds)
				pDevice->AssignBuffer(batchData.m_WorldsBuffer, batchData.m_Worlds->data(), batchData.m_Worlds->size()*sizeof(glm::mat4));

			if (!batchData.m_MaterialsBuffer)
			{
				batchData.m_MaterialsBuffer = pDevice->CreateBuffer(batchData.m_Worlds->size()*sizeof(glm::mat4), BT_Storage);
				batchData.m_MaterialDatas.m_Dirty = true;
			}
			if (batchData.m_MaterialDatas)
				pDevice->AssignBuffer(batchData.m_MaterialsBuffer, batchData.m_MaterialDatas->data(), batchData.m_MaterialDatas->size());

			if (textureCount && !batchData.m_TextureBitsBuffer)
			{
				batchData.m_TextureBitsBuffer = pDevice->CreateBuffer(batchData.m_TextureBits->size()*sizeof(uint32_t), BT_Storage);
				batchData.m_TextureBits.m_Dirty = true;
			}
			if (textureCount && batchData.m_TextureBits)
				pDevice->AssignBuffer(batchData.m_TextureBitsBuffer, batchData.m_TextureBits->data(), batchData.m_TextureBits->size()*sizeof(uint32_t));

			if (!batchData.m_Set)
			{
				DescriptorSetLayoutInfo setLayoutInfo;
				DescriptorSetInfo setInfo;
				setInfo.m_Buffers.resize(2 + (textureCount > 0 ? 1 : 0));
				setLayoutInfo.m_Buffers.resize(2 + (textureCount > 0 ? 1 : 0));
				setLayoutInfo.m_Buffers[0].m_BindingIndex = uint32_t(BindingIndices::WorldTransforms);
				setLayoutInfo.m_Buffers[0].m_Type = BT_Storage;
				setLayoutInfo.m_Buffers[0].m_ShaderStages = STF_Vertex;
				setInfo.m_Buffers[0].m_BufferHandle = batchData.m_WorldsBuffer;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = batchData.m_Worlds->size()*sizeof(glm::mat4);

				setLayoutInfo.m_Buffers[1].m_BindingIndex = uint32_t(BindingIndices::Materials);
				setLayoutInfo.m_Buffers[1].m_Type = BT_Storage;
				setLayoutInfo.m_Buffers[1].m_ShaderStages = STF_Fragment;
				setInfo.m_Buffers[1].m_BufferHandle = batchData.m_MaterialsBuffer;
				setInfo.m_Buffers[1].m_Offset = 0;
				setInfo.m_Buffers[1].m_Size = batchData.m_MaterialDatas->size();
				if (batchData.m_TextureBitsBuffer)
				{
					setLayoutInfo.m_Buffers[2].m_BindingIndex = uint32_t(BindingIndices::HasTexture);
					setLayoutInfo.m_Buffers[2].m_Type = BT_Storage;
					setLayoutInfo.m_Buffers[2].m_ShaderStages = STF_Fragment;
					setInfo.m_Buffers[2].m_BufferHandle = batchData.m_TextureBitsBuffer;
					setInfo.m_Buffers[2].m_Offset = 0;
					setInfo.m_Buffers[2].m_Size = batchData.m_TextureBits->size()*sizeof(uint32_t);
				}

				batchData.m_SetLayout = setInfo.m_Layout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
				batchData.m_Set = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!batchData.m_Pipeline)
			{
				std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts(textureCount ? 3 : 2);
				descriptorSetLayouts[0] = m_GlobalSetLayout;
				descriptorSetLayouts[1] = batchData.m_SetLayout;
				if (textureCount) descriptorSetLayouts[2] = batchData.m_TextureSetLayout;

				PipelineData* pPipelineData = pipelines.GetPipelineData(pipelineBatch.m_PipelineID);
				if (!pPipelineData) continue;
				Resource* pMeshResource = assets.FindResource(pipelineBatch.m_UniqueMeshOrder[0]);
				if (!pMeshResource) continue;
				MeshData* pMesh = static_cast<MeshData*>(pMeshResource);
				batchData.m_Pipeline = pDevice->AcquireCachedPipeline(defaultRenderPass, pPipelineData,
					std::move(descriptorSetLayouts), pMesh->VertexSize(), pMesh->AttributeTypesVector());
			}
		}
	}

	void GloryRendererModule::GenerateClusterSSBO(GraphicsDevice* pDevice, CameraRef camera, DescriptorSetHandle clusterSet)
	{
		const glm::uvec2 resolution = camera.GetResolution();
		const glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		const float zNear = camera.GetNear();
		const float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x/(float)gridSize.x), (uint32_t)std::ceilf(resolution.y/(float)gridSize.y));
		ScreenToView screenToView;
		screenToView.ProjectionInverse = camera.GetProjectionInverse();
		screenToView.ViewInverse = camera.GetViewInverse();
		screenToView.ScreenDimensions = resolution;
		screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		screenToView.Scale = (float)gridSize.z/std::log2f(zFar/zNear);
		screenToView.Bias = -((float)gridSize.z*std::log2f(zNear)/std::log2f(zFar/zNear));
		screenToView.zNear = zNear;
		screenToView.zFar = zFar;

		pDevice->AssignBuffer(m_ScreenToViewBuffer, (void*)&screenToView, sizeof(ScreenToView));

		CommandBufferHandle commandBuffer = pDevice->Begin();
		pDevice->BeginPipeline(commandBuffer, m_ClusterPipeline);
		pDevice->BindDescriptorSets(commandBuffer, m_ClusterPipeline, { clusterSet });
		pDevice->Dispatch(commandBuffer, gridSize.x, gridSize.y, gridSize.z);
		pDevice->EndPipeline(commandBuffer);
		pDevice->End(commandBuffer);
		pDevice->Commit(commandBuffer);
	}

	void GloryRendererModule::ClusterPass(uint32_t cameraIndex)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();

		CameraRef camera = m_FrameData.ActiveCameras[cameraIndex];
		DescriptorSetHandle& clusterCullSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("ClusterCullSet"));

		glm::uvec2 resolution = camera.GetResolution();
		glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		BufferHandle clusterSSBO = camera.GetUserHandle("ClusterSSBO");
		BufferHandle lightIndexSSBO = camera.GetUserHandle("LightIndexSSBO");
		BufferHandle lightGridSSBO = camera.GetUserHandle("LightGridSSBO");
		if (!clusterSSBO || !lightIndexSSBO || !lightGridSSBO) return;

		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x/(float)gridSize.x), (uint32_t)std::ceilf(resolution.y/(float)gridSize.y));
		ScreenToView screenToView;
		screenToView.ProjectionInverse = camera.GetProjectionInverse();
		screenToView.ViewInverse = camera.GetViewInverse();
		screenToView.ScreenDimensions = resolution;
		screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		screenToView.Scale = (float)gridSize.z/std::log2f(zFar/zNear);
		screenToView.Bias = -((float)gridSize.z*std::log2f(zNear)/std::log2f(zFar/zNear));
		screenToView.zNear = zNear;
		screenToView.zFar = zFar;

		pDevice->AssignBuffer(m_ScreenToViewBuffer, (void*)&screenToView, sizeof(ScreenToView));

		CommandBufferHandle commandBuffer = pDevice->Begin();
		pDevice->BeginPipeline(commandBuffer, m_ClusterCullLightPipeline);
		pDevice->BindDescriptorSets(commandBuffer, m_ClusterCullLightPipeline, { clusterCullSet });
		pDevice->Dispatch(commandBuffer, 1, 1, 6);
		pDevice->EndPipeline(commandBuffer);
		pDevice->End(commandBuffer);
		pDevice->Commit(commandBuffer);
		pDevice->Wait(commandBuffer);
		pDevice->Release(commandBuffer);
	}

	void GloryRendererModule::DynamicObjectsPass(uint32_t cameraIndex)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		CameraRef camera = m_FrameData.ActiveCameras[cameraIndex];
		//pGraphics->EnableDepthWrite(true);
		RenderBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData, cameraIndex);
		//pGraphics->EnableDepthWrite(true);
	}
}
