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
		HasTexture = 5,
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

		m_GlobalSetLayout = setInfo.m_Layout = pDevice->CreateDescriptorSetLayout(setLayoutInfo);
		m_GlobalSet = pDevice->CreateDescriptorSet(std::move(setInfo));
	}

	void NullRendererModule::Update()
	{
	}

	//void NullRendererModule::Draw()
	//{
	//	GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
	//	if (!pDevice) return;

	//	AssetManager& assets = m_pEngine->GetAssetManager();
	//	MaterialManager& materials = m_pEngine->GetMaterialManager();

	//	pDevice->Begin();
	//	for (size_t i = 0; i < m_FrameData.ActiveCameras.size(); ++i)
	//	{
	//		CameraRef camera = m_FrameData.ActiveCameras[i];
	//		RenderPassHandle& renderPass = reinterpret_cast<UUID&>(camera.GetUserHandle("RenderPass"));
	//		if (!renderPass)
	//		{
	//			RenderPassInfo renderPassInfo;
	//			renderPassInfo.RenderTextureInfo.Width = camera.GetResolution().x;
	//			renderPassInfo.RenderTextureInfo.Height = camera.GetResolution().y;
	//			renderPassInfo.RenderTextureInfo.HasDepth = true;
	//			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("object", PixelFormat::PF_RGBAI, PixelFormat::PF_R32G32B32A32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UInt, false));
	//			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Debug", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
	//			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
	//			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R16G16B16A16Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
	//			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("AOBlurred", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
	//			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Data", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
	//			renderPass = pDevice->CreateRenderPass(renderPassInfo);

	//			//FileImportSettings importSettings;
	//			//importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
	//			//importSettings.AddNullTerminateAtEnd = false;

	//			//std::filesystem::path path;
	//			//FileData* pVertShader = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("Triangle_vert.spv", importSettings);
	//			//FileData* pFragShader = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("Triangle_frag.spv", importSettings);

	//			//FileData* pVertShader = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("Triangle_vert.shader", importSettings);
	//			//FileData* pFragShader = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("Triangle_frag.shader", importSettings);

	//			//InternalPipeline pipeline({ pVertShader, pFragShader }, { ShaderType::ST_Vertex, ShaderType::ST_Fragment });

	//			//m_Pipeline = pDevice->CreatePipeline(renderPass, &pipeline, sizeof(glm::vec2), {AttributeType::Float2});
	//		}
	//		pDevice->BeginRenderPass(renderPass);

	//		/*for (RenderData& renderData : m_FrameData.ObjectsToRender)
	//		{
	//			Resource* pMaterialResource = m_pEngine->GetAssetManager().FindResource(renderData.m_MaterialID);
	//			if (!pMaterialResource) continue;
	//			Resource* pMeshResource = m_pEngine->GetAssetManager().FindResource(renderData.m_MeshID);
	//			if (!pMeshResource) continue;
	//			MaterialData* pMaterialData = static_cast<MaterialData*>(pMaterialResource);
	//			MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
	//			PipelineData* pPipelineData = pMaterialData->GetPipeline(m_pEngine->GetMaterialManager(), m_pEngine->GetPipelineManager());
	//			if (!pPipelineData) continue;

	//			PipelineHandle pipeline = pDevice->AcquireCachedPipeline(renderPass, pPipelineData, sizeof(DefaultVertex3D), { AttributeType::Float3, AttributeType::Float3,
	//				AttributeType::Float3, AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });

	//			const std::vector<char>& propertiesData = pMaterialData->GetBufferReference(materials);
	//			BufferHandle propertiesBuffer = pDevice->AcquireCachedPipelineBuffer(pipeline, 1, propertiesData.size(), BufferType::BT_Storage);
	//			BufferHandle objectDataBuffer = pDevice->AcquireCachedPipelineBuffer(pipeline, 2, sizeof(ObjectData), BufferType::BT_Storage);
	//			pDevice->AssignBuffer(propertiesBuffer, propertiesData.data());

	//			MeshHandle mesh = pDevice->AcquireCachedMesh(pMeshData);

	//			pDevice->BeginPipeline(pipeline);
	//			pDevice->DrawMesh(mesh);
	//			pDevice->EndPipeline();
	//		}*/
	//		pDevice->EndRenderPass();
	//	}
	//	pDevice->End();
	//}

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
					//Material* pMaterial = pDevice->CreateMaterial(pMaterialData);
					//if (!pMaterial) continue;

					const auto& ids = meshBatch.m_ObjectIDs[i];
					//pMaterial->Reset();
					constants.m_ObjectID = ids.second;
					constants.m_SceneID = ids.first;
					constants.m_ObjectDataIndex = currentObject;
					constants.m_MaterialIndex = meshBatch.m_MaterialIndices[i];

					if (usePushConstants)
						pDevice->PushConstants(batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants);
					else
						pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));
					//pMaterial->SetSamplers(m_pEngine);
					//pMaterial->SetTextureBitsBuffer(m_pEngine);
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
		//
		//if (m_LightCameraDatas->size() < m_FrameData.LightSpaceTransforms.count()) m_LightCameraDatas.resize(m_FrameData.LightSpaceTransforms.count());
		//for (size_t i = 0; i < m_FrameData.LightSpaceTransforms.count(); ++i)
		//{
		//	m_LightCameraDatas.m_Data[i].m_View = glm::identity<glm::mat4>();
		//	if (m_LightCameraDatas.m_Data[i].m_Projection != m_FrameData.LightSpaceTransforms[i])
		//	{
		//		m_LightCameraDatas.m_Data[i].m_Projection = m_FrameData.LightSpaceTransforms[i];
		//		m_LightCameraDatas.m_Dirty = true;
		//	}
		//}
		//if (m_LightCameraDatas)
		//	m_pLightCameraDatasBuffer->Assign(m_LightCameraDatas->data(), m_LightCameraDatas->size() * sizeof(PerCameraData));

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

				/* Textures */
				for (size_t i = 0; i < pMaterialData->ResourceCount(); ++i)
				{
					const UUID textureID = pMaterialData->GetResourceUUIDPointer(i)->AssetUUID();
					Resource* pResource = assets.FindResource(textureID);
					if (!pResource) continue;
					TextureData* pTexture = static_cast<TextureData*>(pResource);
					TextureHandle texture = pDevice->AcquireCachedTexture(pTexture);
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
				batchData.m_SetLayout = setInfo.m_Layout = pDevice->CreateDescriptorSetLayout(setLayoutInfo);
				batchData.m_Set = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!batchData.m_Pipeline)
			{
				std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts(2);
				descriptorSetLayouts[0] = m_GlobalSetLayout;
				descriptorSetLayouts[1] = batchData.m_SetLayout;

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
}
