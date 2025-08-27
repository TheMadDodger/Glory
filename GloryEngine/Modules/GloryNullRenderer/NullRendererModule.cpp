#include "NullRendererModule.h"

#include <Engine.h>
#include <GraphicsDevice.h>
#include <MeshData.h>
#include <PipelineManager.h>
#include <MaterialManager.h>

#include <FileLoaderModule.h>
#include <InternalPipeline.h>
#include <AssetManager.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(NullRendererModule);

	enum class BindingIndices : uint32_t
	{
		RenderConstants = 1,
		CameraDatas = 2,
		WorldTransforms = 3,
		Materials = 4,
		HasTexture = 6,
	};

	NullRendererModule::NullRendererModule()
	{
	}

	NullRendererModule::~NullRendererModule()
	{
	}

	void NullRendererModule::GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos)
	{
	}

	void NullRendererModule::OnCameraResize(CameraRef camera)
	{
	}

	void NullRendererModule::OnCameraPerspectiveChanged(CameraRef camera)
	{
	}

	MaterialData* NullRendererModule::GetInternalMaterial(std::string_view name) const
	{
		return nullptr;
	}

	void NullRendererModule::CollectReferences(std::vector<UUID>& references)
	{
	}

	UUID NullRendererModule::TextPipelineID() const
	{
		return 0;
	}

	void NullRendererModule::Initialize()
	{
		RendererModule::Initialize();

		AddRenderPass(RP_Prepass, RenderPass{ "Prepare Data Pass", [this](uint32_t, RendererModule*) {
			PrepareDataPass();
		} });

		m_RenderPasses[RP_ObjectPass].push_back(RenderPass{ "Dynamic Object Pass", [this](uint32_t cameraIndex, RendererModule*) {
			DynamicObjectsPass(cameraIndex);
		} });
	}

	void NullRendererModule::OnPostInitialize()
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice)
		{
			m_pEngine->GetDebug().LogError("Renderer: No graphics device active");
			return;
		}

		FileImportSettings importSettings;
		importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
		importSettings.AddNullTerminateAtEnd = true;

		// Cluster generator shader
		std::filesystem::path path;
		GetResourcePath("Shaders/Compute/ClusterShader.shader", path);
		m_pClusterShaderData = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pClusterShaderPipelineData = new InternalPipeline({ m_pClusterShaderData }, { ShaderType::ST_Compute });

		//RenderPassInfo clusterGeneratorPassInfo;
		//RenderPassHandle clusterGeneratorPass = pDevice->CreateRenderPass(clusterGeneratorPassInfo);
		//pDevice->CreatePipeline(clusterGeneratorPass, );

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
		}
		else
		{
			setLayoutInfo.m_PushConstantRange.m_Offset = 0;
			setLayoutInfo.m_PushConstantRange.m_Size = sizeof(RenderConstants);
		}
		const size_t cameraDatasBufferIndex = usePushConstants ? 0 : 1;
		m_CameraDatasBuffer = setInfo.m_Buffers[cameraDatasBufferIndex].m_BufferHandle = pDevice->CreateBuffer(sizeof(PerCameraData)*MAX_CAMERAS, BufferType::BT_Uniform);
		setLayoutInfo.m_Buffers[cameraDatasBufferIndex].m_BindingIndex = uint32_t(BindingIndices::CameraDatas);
		setLayoutInfo.m_Buffers[cameraDatasBufferIndex].m_Type = BufferType::BT_Uniform;
		setInfo.m_Buffers[cameraDatasBufferIndex].m_Offset = 0;
		setInfo.m_Buffers[cameraDatasBufferIndex].m_Size = sizeof(PerCameraData)*MAX_CAMERAS;
		//m_LightCameraDatasBuffer = pDevice->CreateBuffer(CameraDatasBufferName, sizeof(PerCameraData)*MAX_LIGHTS, BufferType::BT_Uniform);

		m_GlobalSetLayout = setInfo.m_Layout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
		m_GlobalSet = pDevice->CreateDescriptorSet(std::move(setInfo));

		setLayoutInfo = DescriptorSetLayoutInfo();
		setLayoutInfo.m_Buffers.resize(2);
		setLayoutInfo.m_Buffers[0].m_BindingIndex = 1;
		setLayoutInfo.m_Buffers[0].m_Type = BufferType::BT_Storage;
		setLayoutInfo.m_Buffers[1].m_BindingIndex = 2;
		setLayoutInfo.m_Buffers[1].m_Type = BufferType::BT_Storage;
		m_ClusterSetLayout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
		m_ClusterPipeline = pDevice->CreateComputePipeline(m_pClusterShaderPipelineData, { m_ClusterSetLayout });

		//m_LightsSSBO = pDevice->CreateBuffer(sizeof(LightData)*MAX_LIGHTS, BufferType::BT_Storage);
		//m_LightCountSSBO = pDevice->CreateBuffer(sizeof(uint32_t), BufferType::BT_Storage);
		//m_LightSpaceTransformsSSBO = pDevice->CreateBuffer(sizeof(glm::mat4)*MAX_LIGHTS, BufferType::BT_Storage);
		//m_LightDistancesSSBO = pDevice->CreateBuffer(sizeof(uint32_t)*MAX_LIGHTS, BufferType::BT_Storage);
	}

	void NullRendererModule::Update()
	{
	}

	void NullRendererModule::Cleanup()
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;
	}

	void NullRendererModule::OnRenderEffects(CameraRef camera, RenderTexture* pRenderTexture)
	{
	}

	void NullRendererModule::OnDoCompositing(CameraRef camera, uint32_t width, uint32_t height, RenderTexture* pRenderTexture)
	{
	}

	void NullRendererModule::OnDisplayCopy(RenderTexture* pRenderTexture, uint32_t width, uint32_t height)
	{
	}

	void NullRendererModule::OnRenderSkybox(CameraRef camera, CubemapData* pCubemap)
	{
	}

	void NullRendererModule::OnStartCameraRender(CameraRef camera, const FrameData<LightData>& lights)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();

		BufferHandle& clusterSSBOHandle = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("ClusterSSBO"));
		if (!clusterSSBOHandle)
		{
			BufferHandle& lightIndexSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightIndexSSBO"));
			BufferHandle& lightGridSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightGridSSBO"));

			clusterSSBOHandle = pDevice->CreateBuffer(sizeof(VolumeTileAABB)*NUM_CLUSTERS, BufferType::BT_Storage);
			lightIndexSSBO = pDevice->CreateBuffer(sizeof(uint32_t)*(NUM_CLUSTERS * MAX_LIGHTS_PER_TILE + 1), BufferType::BT_Storage);
			lightGridSSBO = pDevice->CreateBuffer(sizeof(LightGrid)*NUM_CLUSTERS, BufferType::BT_Storage);

			//GenerateClusterSSBO(pClusterSSBO, camera);
		}
	}

	void NullRendererModule::OnEndCameraRender(CameraRef camera, const FrameData<LightData>& lights)
	{
	}

	void NullRendererModule::LoadSettings(ModuleSettings& settings)
	{
		RendererModule::LoadSettings(settings);
	}

	void NullRendererModule::RenderBatches(const std::vector<PipelineBatch>& batches, const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		MaterialManager& materialManager = m_pEngine->GetMaterialManager();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		AssetManager& assets = m_pEngine->GetAssetManager();

		const bool usePushConstants = pDevice->IsSupported(APIFeatures::PushConstants);

		RenderConstants constants;
		constants.m_CameraIndex = cameraIndex;
		if (!usePushConstants) pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));

		size_t batchIndex = 0;
		for (const PipelineBatch& pipelineRenderData : batches)
		{
			if (batchIndex >= batchDatas.size()) break;
			const PipelineBatchData& batchData = batchDatas.at(batchIndex);
			++batchIndex;

			PipelineData* pPipelineData = pipelines.GetPipelineData(pipelineRenderData.m_PipelineID);
			if (!pPipelineData) continue;
			pDevice->BeginPipeline(batchData.m_Pipeline);
			pDevice->BindDescriptorSets(batchData.m_Pipeline, { m_GlobalSet, batchData.m_Set });

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
						pDevice->PushConstants(batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants);
					else
						pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));
					if (!batchData.m_TextureSets.empty())
						pDevice->BindDescriptorSets(batchData.m_Pipeline, { batchData.m_TextureSets[constants.m_MaterialIndex] }, 2);
					pDevice->DrawMesh(mesh);
				}
			}

			/*const size_t materialCount = pipelineRenderData.m_UniqueMaterials->size();
			MaterialData* pBaseMaterialData = materialManager.GetMaterial(pipelineRenderData.m_UniqueMaterials.m_Data[0]);
			if (!pBaseMaterialData) continue;
			const size_t propertyDataSize = pBaseMaterialData->PropertyDataSize(materialManager);
			const size_t paddingBytes = 16 - propertyDataSize%16;
			const size_t finalPropertyDataSize = propertyDataSize + paddingBytes;
			pipelineRenderData.m_PropertiesBuffer.resize(finalPropertyDataSize*materialCount);

			for (size_t i = 0; i < materialCount; ++i)
			{
				MaterialData* pMaterialData = materialManager.GetMaterial(pipelineRenderData.m_UniqueMaterials.m_Data[i]);
				if (std::memcmp(&pipelineRenderData.m_PropertiesBuffer.m_Data[i*finalPropertyDataSize],
					pMaterialData->GetFinalBufferReference(materialManager).data(), propertyDataSize) == 0)
					continue;

				pMaterialData->CopyProperties(materialManager, &pipelineRenderData.m_PropertiesBuffer.m_Data[i*finalPropertyDataSize]);
				pipelineRenderData.m_PropertiesBuffer.m_Dirty = true;
			}

			if (pipelineRenderData.m_PropertiesBuffer)
			{
				pipelineRenderData.m_pIndirectMaterialPropertyData->Assign(pipelineRenderData.m_PropertiesBuffer->data(),
					pipelineRenderData.m_PropertiesBuffer->size());
			}*/

			pDevice->EndPipeline();
		}
	}

	void NullRendererModule::PrepareDataPass()
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		MaterialManager& materials = m_pEngine->GetMaterialManager();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();

		/* Prepare cameras */
		if (m_CameraDatas->size() < m_FrameData.ActiveCameras.size())
			m_CameraDatas.resize(m_FrameData.ActiveCameras.size());
		for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
		{
			const PerCameraData cameraData{ m_FrameData.ActiveCameras[i].GetView(),
				m_FrameData.ActiveCameras[i].GetProjection() };

			if (m_CameraDatas.m_Data[i].m_Projection != cameraData.m_Projection ||
				m_CameraDatas.m_Data[i].m_View != cameraData.m_View)
			{
				std::memcpy(&m_CameraDatas.m_Data[i], &cameraData, sizeof(PerCameraData));
				m_CameraDatas.m_Dirty = true;
			}
		}
		if (m_CameraDatas)
			pDevice->AssignBuffer(m_CameraDatasBuffer, m_CameraDatas->data(), m_CameraDatas->size()*sizeof(PerCameraData));

		/* Update light data */
		//const uint32_t count = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		//m_LightCountSSBO->Assign(&count, 0, sizeof(uint32_t));
		//m_pLightsSSBO->Assign(m_FrameData.ActiveLights.data(), 0, MAX_LIGHTS * sizeof(LightData));
		//m_pLightSpaceTransformsSSBO->Assign(m_FrameData.LightSpaceTransforms.data(), 0, MAX_LIGHTS * sizeof(glm::mat4));
		//m_pLightDistancesSSBO->Assign(ResetLightDistances);
		
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
			//m_pLightCameraDatasBuffer->Assign(m_LightCameraDatas->data(), m_LightCameraDatas->size() * sizeof(PerCameraData));

		PrepareBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData);
	}

	void NullRendererModule::PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas)
	{
		if (m_FrameData.ActiveCameras.empty()) return;
		CameraRef defaultCamera = m_FrameData.ActiveCameras[0];
		RenderPassHandle& defaultRenderPass = reinterpret_cast<UUID&>(defaultCamera.GetUserHandle("RenderPass"));
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
			const size_t paddingBytes = 16 - propertyDataSize%16;
			const size_t finalPropertyDataSize = propertyDataSize + paddingBytes;
			const size_t totalBufferSize = finalPropertyDataSize * pipelineBatch.m_UniqueMaterials.size();
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
					texturesSetLayoutInfo.m_Samplers[i].m_BindingIndex = i;
				}
				batchData.m_TextureSetLayout = pDevice->CreateDescriptorSetLayout(std::move(texturesSetLayoutInfo));
				batchData.m_TextureSets.resize(pipelineBatch.m_UniqueMaterials.size(), 0ull);
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
				setInfo.m_Buffers[0].m_BufferHandle = batchData.m_WorldsBuffer;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = batchData.m_Worlds->size()*sizeof(glm::mat4);

				setLayoutInfo.m_Buffers[1].m_BindingIndex = uint32_t(BindingIndices::Materials);
				setLayoutInfo.m_Buffers[1].m_Type = BT_Storage;
				setInfo.m_Buffers[1].m_BufferHandle = batchData.m_MaterialsBuffer;
				setInfo.m_Buffers[1].m_Offset = 0;
				setInfo.m_Buffers[1].m_Size = batchData.m_MaterialDatas->size();
				if (batchData.m_TextureBitsBuffer)
				{
					setLayoutInfo.m_Buffers[2].m_BindingIndex = uint32_t(BindingIndices::HasTexture);
					setLayoutInfo.m_Buffers[2].m_Type = BT_Storage;
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

	void NullRendererModule::DynamicObjectsPass(uint32_t cameraIndex)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();

		CameraRef camera = m_FrameData.ActiveCameras[cameraIndex];

		//pGraphics->EnableDepthWrite(true);
		RenderBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData, cameraIndex);
		//pGraphics->EnableDepthWrite(true);
	}

	void NullRendererModule::GenerateClusterSSBO(Buffer* pBuffer, CameraRef camera)
	{
		//const glm::uvec2 resolution = camera.GetResolution();
		//const glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);
		//
		//const float zNear = camera.GetNear();
		//const float zFar = camera.GetFar();
		//
		//const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x / (float)gridSize.x), (uint32_t)std::ceilf(resolution.y / (float)gridSize.y));
		//ScreenToView screenToView;
		//screenToView.ProjectionInverse = camera.GetProjectionInverse();
		//screenToView.ViewInverse = camera.GetViewInverse();
		//screenToView.ScreenDimensions = resolution;
		//screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		//screenToView.Scale = (float)gridSize.z / std::log2f(zFar / zNear);
		//screenToView.Bias = -((float)gridSize.z * std::log2f(zNear) / std::log2f(zFar / zNear));
		//screenToView.zNear = zNear;
		//screenToView.zFar = zFar;
		//
		//m_pScreenToViewSSBO->Assign((void*)&screenToView);
		//
		//m_pClusterShaderMaterial->Use();
		//pBuffer->BindForDraw();
		//m_pScreenToViewSSBO->BindForDraw();
		//m_pEngine->GetMainModule<GraphicsModule>()->DispatchCompute(gridSize.x, gridSize.y, gridSize.z);
		//pBuffer->Unbind();
		//m_pScreenToViewSSBO->Unbind();
	}
}
