#include "GloryRendererModule.h"

#include <Engine.h>
#include <Console.h>
#include <GraphicsDevice.h>
#include <GPUTextureAtlas.h>

#include <PipelineManager.h>
#include <MaterialManager.h>
#include <AssetManager.h>

#include <PipelineData.h>
#include <MeshData.h>
#include <TextureData.h>

#include <EngineProfiler.h>
#include <random>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Glory
{
	static constexpr std::string_view ScreenSpaceAOCVarName = "r_screenSpaceAO";
	static constexpr std::string_view MinShadowResolutionVarName = "r_minShadowResolution";
	static constexpr std::string_view MaxShadowResolutionVarName = "r_maxShadowResolution";
	static constexpr std::string_view ShadowAtlasResolution = "r_shadowAtlasResolution";
	static constexpr std::string_view MaxShadowLODs = "r_maxShadowLODs";

	static uint32_t* ResetLightDistances;

	constexpr size_t AttachmentNameCount = 5;
	constexpr std::string_view AttachmentNames[AttachmentNameCount] = {
		"ObjectID",
		//"Debug",
		"Color",
		"Normal",
		//"AOBlurred",
		//"Data",
		"AO",
		"Final",
	};

	constexpr size_t DebugOverlayNameCount = 1;
	constexpr std::string_view DebugOverlayNames[AttachmentNameCount] = {
		"Shadow Atlas",
	};

	constexpr size_t MaxPicks = 8;

	GLORY_MODULE_VERSION_CPP(GloryRendererModule);

	struct BufferBindingIndices
	{
		static constexpr uint32_t RenderConstants = 0;
		static constexpr uint32_t CameraDatas = 1;
		static constexpr uint32_t WorldTransforms = 2;
		static constexpr uint32_t LightDatas = 3;
		static constexpr uint32_t LightSpaceTransforms = 4;
		static constexpr uint32_t LightIndices = 5;
		static constexpr uint32_t LightGrid = 6;
		static constexpr uint32_t LightDistances = 7;
		static constexpr uint32_t PickingResults = 2;

		static constexpr uint32_t Materials = 8;
		static constexpr uint32_t HasTexture = 9;

		static constexpr uint32_t Clusters = 2;
		static constexpr uint32_t SampleDome = 2;
	};

	GloryRendererModule::GloryRendererModule(): m_MinShadowResolution(256), m_MaxShadowResolution(2048),
		m_ShadowAtlasResolution(8192), m_ShadowMapResolutions{}, m_MaxShadowLODs(6)
	{
	}

	GloryRendererModule::~GloryRendererModule()
	{
	}

	void GloryRendererModule::OnCameraResize(CameraRef camera)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;
		pDevice->WaitIdle();

		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		for (size_t i = 0; i < m_ImageCount; ++i)
		{
			const RenderPassHandle& renderPass = uniqueCameraData.m_RenderPasses[i];
			const RenderPassHandle& ssaoRenderPass = uniqueCameraData.m_SSAORenderPasses[i];
			if (!renderPass) continue;
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			RenderTextureHandle ssaoRenderTexture = pDevice->GetRenderPassRenderTexture(ssaoRenderPass);
			const glm::uvec2 resolution = camera.GetResolution();
			pDevice->ResizeRenderTexture(renderTexture, resolution.x, resolution.y);
			pDevice->ResizeRenderTexture(ssaoRenderTexture, resolution.x, resolution.y);

			/* Update descriptor sets */
			const DescriptorSetHandle& ssaoSamplersSet = uniqueCameraData.m_SSAOSamplersSets[i];
			const DescriptorSetHandle& colorSamplerSet = uniqueCameraData.m_FinalColorSamplerSets[i];
			const DescriptorSetHandle& pickingSamplersSet = uniqueCameraData.m_PickingSamplersSets[i];
			TextureHandle objectID = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 1);
			TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 2);
			TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 3);

			DescriptorSetUpdateInfo updateInfo;
			updateInfo.m_Samplers.resize(2);
			updateInfo.m_Samplers[0].m_TextureHandle = normals;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			updateInfo.m_Samplers[1].m_TextureHandle = depth;
			updateInfo.m_Samplers[1].m_DescriptorIndex = 1;
			pDevice->UpdateDescriptorSet(ssaoSamplersSet, updateInfo);

			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = color;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(colorSamplerSet, updateInfo);

			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(3);
			updateInfo.m_Samplers[0].m_TextureHandle = objectID;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			updateInfo.m_Samplers[1].m_TextureHandle = normals;
			updateInfo.m_Samplers[1].m_DescriptorIndex = 1;
			updateInfo.m_Samplers[2].m_TextureHandle = depth;
			updateInfo.m_Samplers[2].m_DescriptorIndex = 2;
			pDevice->UpdateDescriptorSet(pickingSamplersSet, updateInfo);
		}

		/* When the camera rendertexture resizes we need to generate a new grid of clusters for that camera */
		OnCameraPerspectiveChanged(camera);
		camera.SetResolutionDirty(false);
	}

	void GloryRendererModule::OnCameraPerspectiveChanged(CameraRef camera)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		/* When the camera changed perspective we need to generate a new grid of clusters for that camera */
		auto iter = std::find_if(m_ActiveCameras.begin(), m_ActiveCameras.end(),
			[camera](const CameraRef& other) { return other.GetUUID() == camera.GetUUID(); });
		if (iter == m_ActiveCameras.end()) return;
		const size_t cameraIndex = iter - m_ActiveCameras.begin();
		pDevice->WaitIdle();
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		if (!uniqueCameraData.m_ClusterSet) return; // Should not happen but just in case
		GenerateClusterSSBO(cameraIndex, pDevice, camera, uniqueCameraData.m_ClusterSet);
		m_DirtyCameraPerspectives.push_back(camera);
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
		newReferences.push_back(settings.Value<uint64_t>("Picking"));

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

	void GloryRendererModule::PresentFrame()
	{
		if (!m_Enabled) return;

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice || !m_Swapchain) return;
		const GraphicsDevice::SwapchainResult result = pDevice->Present(m_Swapchain, m_CurrentFrameIndex, { m_RenderingFinishedSemaphores[m_CurrentSemaphoreIndex] });
		if (result == GraphicsDevice::SwapchainResult::S_OutOfDate)
		{
			OnWindowResized();
			return;
		}

		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_ImageCount;
		m_CurrentSemaphoreIndex = (m_CurrentSemaphoreIndex + 1) % m_ImageCount;
	}

	void GloryRendererModule::Initialize()
	{
		RendererModule::Initialize();
		m_pEngine->GetConsole().RegisterCVar({ std::string{ ScreenSpaceAOCVarName }, "Enables/disables screen space ambient occlusion.", float(m_GlobalSSAOSetting.m_Enabled), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MinShadowResolutionVarName }, "Sets the minimum resolution for shadow maps.", float(m_MinShadowResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MaxShadowResolutionVarName }, "Sets the maximum resolution for shadow maps.", float(m_MaxShadowResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ ShadowAtlasResolution }, "Sets the resolution for the shadow atlas.", float(m_ShadowAtlasResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MaxShadowLODs }, "Sets the number of shadow map LODs.", float(m_MaxShadowLODs), CVar::Flags::Save });

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ ScreenSpaceAOCVarName }, [this](const CVar* cvar) {
			m_GlobalSSAOSetting.m_Enabled = cvar->m_Value == 1.0f;
			m_GlobalSSAOSetting.m_Dirty = true;
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ MinShadowResolutionVarName }, [this](const CVar* cvar) {
			ResizeShadowMapLODResolutions(uint32_t(cvar->m_Value), m_MaxShadowResolution);
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ MaxShadowResolutionVarName }, [this](const CVar* cvar) {
			ResizeShadowMapLODResolutions(m_MinShadowResolution, uint32_t(cvar->m_Value));
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ MaxShadowLODs }, [this](const CVar* cvar) {
			m_MaxShadowLODs = cvar->m_Value;
			GenerateShadowLODDivisions(cvar->m_Value);
			ResizeShadowMapLODResolutions(m_MinShadowResolution, m_MaxShadowResolution);
		});
	}

	void CreateBufferDescriptorLayoutAndSet(GraphicsDevice* pDevice, bool usePushConstants, size_t numBuffers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<BufferType>& bufferTypes,
		const std::vector<ShaderTypeFlag>& shaderStages, const std::vector<BufferHandle>& bufferHandles,
		const std::vector<std::pair<uint32_t, uint32_t>>& bufferOffsetsAndSizes, DescriptorSetLayoutHandle& layout,
		DescriptorSetHandle& set, BufferHandle* pConstantsBuffer=nullptr, ShaderTypeFlag constantsShaderStage=ShaderTypeFlag(0),
		uint32_t constantsOffset=0, uint32_t constantsSize=0)
	{
		DescriptorSetLayoutInfo setLayoutInfo = DescriptorSetLayoutInfo();
		DescriptorSetInfo setInfo = DescriptorSetInfo();
		size_t firstBufferindex = 0;
		if (pConstantsBuffer)
		{
			if (!usePushConstants)
			{
				firstBufferindex = 1;
				setLayoutInfo.m_Buffers.resize(numBuffers + 1);
				setInfo.m_Buffers.resize(numBuffers + 1);
				if (!(*pConstantsBuffer))
					*pConstantsBuffer = pDevice->CreateBuffer(constantsSize, BufferType::BT_Uniform, BF_Write);
				setInfo.m_Buffers[0].m_BufferHandle = *pConstantsBuffer;
				setInfo.m_Buffers[0].m_Offset = constantsOffset;
				setInfo.m_Buffers[0].m_Size = constantsSize;
				setLayoutInfo.m_Buffers[0].m_BindingIndex = BufferBindingIndices::RenderConstants;
				setLayoutInfo.m_Buffers[0].m_Type = BufferType::BT_Uniform;
				setLayoutInfo.m_Buffers[0].m_ShaderStages = constantsShaderStage;
			}
			else
			{
				setLayoutInfo.m_Buffers.resize(numBuffers);
				setInfo.m_Buffers.resize(numBuffers);
				setLayoutInfo.m_PushConstantRange.m_Offset = constantsOffset;
				setLayoutInfo.m_PushConstantRange.m_Size = constantsSize;
				setLayoutInfo.m_PushConstantRange.m_ShaderStages = constantsShaderStage;
			}
		}
		else
		{
			setLayoutInfo.m_Buffers.resize(numBuffers);
			setInfo.m_Buffers.resize(numBuffers);
		}

		for (size_t i = 0; i < numBuffers; ++i)
		{
			const uint32_t bindingIndex = i >= bindingIndices.size() ? bindingIndices.back() : bindingIndices[i];
			const BufferType bufferType = i >= bufferTypes.size() ? bufferTypes.back() : bufferTypes[i];
			const ShaderTypeFlag shaderStage = i >= shaderStages.size() ? shaderStages.back() : shaderStages[i];

			const size_t index = firstBufferindex + i;
			setLayoutInfo.m_Buffers[index].m_BindingIndex = bindingIndex;
			setLayoutInfo.m_Buffers[index].m_Type = bufferType;
			setLayoutInfo.m_Buffers[index].m_ShaderStages = shaderStage;
			setInfo.m_Buffers[index].m_BufferHandle = bufferHandles[i];
			setInfo.m_Buffers[index].m_Offset = bufferOffsetsAndSizes[i].first;
			setInfo.m_Buffers[index].m_Size = bufferOffsetsAndSizes[i].second;
		}
		layout = setInfo.m_Layout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
		set = pDevice->CreateDescriptorSet(std::move(setInfo));
	}

	DescriptorSetLayoutHandle CreateBufferDescriptorLayout(GraphicsDevice* pDevice, size_t numBuffers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<BufferType>& bufferTypes,
		const std::vector<ShaderTypeFlag>& shaderStages)
	{
		DescriptorSetLayoutInfo setLayoutInfo = DescriptorSetLayoutInfo();
		setLayoutInfo.m_Buffers.resize(numBuffers);

		for (size_t i = 0; i < numBuffers; ++i)
		{
			const uint32_t bindingIndex = i >= bindingIndices.size() ? bindingIndices.back() : bindingIndices[i];
			const BufferType bufferType = i >= bufferTypes.size() ? bufferTypes.back() : bufferTypes[i];
			const ShaderTypeFlag shaderStage = i >= shaderStages.size() ? shaderStages.back() : shaderStages[i];
			setLayoutInfo.m_Buffers[i].m_BindingIndex = bindingIndex;
			setLayoutInfo.m_Buffers[i].m_Type = bufferType;
			setLayoutInfo.m_Buffers[i].m_ShaderStages = shaderStage;
		}
		return pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
	}

	DescriptorSetHandle CreateBufferDescriptorSet(GraphicsDevice* pDevice, bool usePushConstants, size_t numBuffers,
		const std::vector<BufferHandle>& bufferHandles, const std::vector<std::pair<uint32_t, uint32_t>>& bufferOffsetsAndSizes,
		DescriptorSetLayoutHandle layout, BufferHandle* pConstantsBuffer=nullptr, uint32_t constantsOffset=0, uint32_t constantsSize=0)
	{
		DescriptorSetInfo setInfo = DescriptorSetInfo();
		size_t firstBufferindex = 0;
		if (pConstantsBuffer)
		{
			if (!usePushConstants)
			{
				firstBufferindex = 1;
				setInfo.m_Buffers.resize(numBuffers + 1);
				if (!(*pConstantsBuffer))
					*pConstantsBuffer = pDevice->CreateBuffer(constantsSize, BufferType::BT_Uniform, BF_Write);
				setInfo.m_Buffers[0].m_BufferHandle = *pConstantsBuffer;
				setInfo.m_Buffers[0].m_Offset = constantsOffset;
				setInfo.m_Buffers[0].m_Size = constantsSize;
			}
			else
				setInfo.m_Buffers.resize(numBuffers);
		}
		else
			setInfo.m_Buffers.resize(numBuffers);

		for (size_t i = 0; i < numBuffers; ++i)
		{
			const size_t index = firstBufferindex + i;
			setInfo.m_Buffers[index].m_BufferHandle = bufferHandles[i];
			setInfo.m_Buffers[index].m_Offset = bufferOffsetsAndSizes[i].first;
			setInfo.m_Buffers[index].m_Size = bufferOffsetsAndSizes[i].second;
		}
		setInfo.m_Layout = layout;
		return pDevice->CreateDescriptorSet(std::move(setInfo));
	}

	void CreateSamplerDescriptorLayoutAndSet(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<ShaderTypeFlag>& shaderStages,
		std::vector<std::string>&& samplerNames, const std::vector<TextureHandle>& textureHandles,
		DescriptorSetLayoutHandle& layout, DescriptorSetHandle& set)
	{
		DescriptorSetLayoutInfo setLayoutInfo = DescriptorSetLayoutInfo();
		DescriptorSetInfo setInfo = DescriptorSetInfo();
		setLayoutInfo.m_Samplers.resize(numSamplers);
		setLayoutInfo.m_SamplerNames = std::move(samplerNames);
		setInfo.m_Samplers.resize(numSamplers);

		for (size_t i = 0; i < numSamplers; ++i)
		{
			const uint32_t bindingIndex = i >= bindingIndices.size() ? bindingIndices.back() : bindingIndices[i];
			const ShaderTypeFlag shaderStage = i >= shaderStages.size() ? shaderStages.back() : shaderStages[i];
			setLayoutInfo.m_Samplers[i].m_BindingIndex = bindingIndex;
			setLayoutInfo.m_Samplers[i].m_ShaderStages = shaderStage;
			setInfo.m_Samplers[i].m_TextureHandle = textureHandles[i];
		}
		setInfo.m_Layout = layout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
		set = pDevice->CreateDescriptorSet(std::move(setInfo));
	}

	DescriptorSetLayoutHandle CreateSamplerDescriptorLayout(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<uint32_t>& bindingIndices, const std::vector<ShaderTypeFlag>& shaderStages,
		std::vector<std::string>&& samplerNames)
	{
		DescriptorSetLayoutInfo setLayoutInfo = DescriptorSetLayoutInfo();
		setLayoutInfo.m_Samplers.resize(numSamplers);
		setLayoutInfo.m_SamplerNames = std::move(samplerNames);

		for (size_t i = 0; i < numSamplers; ++i)
		{
			const uint32_t bindingIndex = i >= bindingIndices.size() ? bindingIndices.back() : bindingIndices[i];
			const ShaderTypeFlag shaderStage = i >= shaderStages.size() ? shaderStages.back() : shaderStages[i];
			setLayoutInfo.m_Samplers[i].m_BindingIndex = bindingIndex;
			setLayoutInfo.m_Samplers[i].m_ShaderStages = shaderStage;
		}
		return pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
	}

	DescriptorSetHandle CreateSamplerDescriptorSet(GraphicsDevice* pDevice, size_t numSamplers,
		const std::vector<TextureHandle>& textureHandles, DescriptorSetLayoutHandle layout)
	{
		DescriptorSetInfo setInfo = DescriptorSetInfo();
		setInfo.m_Samplers.resize(numSamplers);

		for (size_t i = 0; i < numSamplers; ++i)
		{
			setInfo.m_Samplers[i].m_TextureHandle = textureHandles[i];
		}
		setInfo.m_Layout = layout;
		return pDevice->CreateDescriptorSet(std::move(setInfo));
	}

	void GloryRendererModule::OnPostInitialize()
	{
		const ModuleSettings& settings = Settings();
		const UUID linesPipeline = settings.Value<uint64_t>("Lines Pipeline");
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
		const UUID pickingPipeline = settings.Value<uint64_t>("Picking");

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice)
		{
			m_pEngine->GetDebug().LogError("Renderer: No graphics device active");
			return;
		}
		const bool usePushConstants = pDevice->IsSupported(APIFeatures::PushConstants);

		/* Global data buffers */
		m_CameraDatasBuffer = pDevice->CreateBuffer(sizeof(PerCameraData)*MAX_CAMERAS, BufferType::BT_Storage, BF_Write);
		m_LightCameraDatasBuffer = pDevice->CreateBuffer(sizeof(PerCameraData)*MAX_LIGHTS, BufferType::BT_Storage, BF_Write);
		m_LightsSSBO = pDevice->CreateBuffer(sizeof(LightData)*MAX_LIGHTS, BufferType::BT_Storage, BF_Write);
		m_LightSpaceTransformsSSBO = pDevice->CreateBuffer(sizeof(glm::mat4)*MAX_LIGHTS, BufferType::BT_Storage, BF_Write);

		GenerateDomeSamplePointsSSBO(pDevice, 64);
		GenerateNoiseTexture(pDevice);

		/* Global set */
		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Vertex | STF_Fragment) }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalRenderSetLayout, m_GlobalRenderSet, &m_RenderConstantsBuffer, ShaderTypeFlag(STF_Vertex | STF_Fragment), 0, sizeof(RenderConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Vertex | STF_Fragment) }, { m_LightCameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_LIGHTS } },
			m_GlobalShadowRenderSetLayout, m_GlobalShadowRenderSet, &m_RenderConstantsBuffer, ShaderTypeFlag(STF_Vertex | STF_Fragment), 0, sizeof(RenderConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Compute }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalPickingSetLayout, m_GlobalPickingSet, &m_PickingConstantsBuffer, STF_Compute, 0, sizeof(PickingConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Vertex }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalLineRenderSetLayout, m_GlobalLineRenderSet, & m_LineRenderConstantsBuffer, STF_Vertex, 0, sizeof(uint32_t));

		assert(m_GlobalRenderSetLayout == m_GlobalShadowRenderSetLayout);

		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalClusterSetLayout, m_GlobalClusterSet, &m_ClusterConstantsBuffer, ShaderTypeFlag(STF_Compute | STF_Fragment), 0, sizeof(ClusterConstants));

		m_CameraClusterSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::Clusters },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) });

		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 2, { BufferBindingIndices::LightDatas, BufferBindingIndices::LightSpaceTransforms },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) }, { m_LightsSSBO, m_LightSpaceTransformsSSBO }, { { 0, sizeof(LightData)*MAX_LIGHTS }, { 0, sizeof(glm::mat4)*MAX_LIGHTS } },
			m_GlobalLightSetLayout, m_GlobalLightSet);

		m_CameraLightSetLayout = CreateBufferDescriptorLayout(pDevice, 3, { BufferBindingIndices::LightIndices, BufferBindingIndices::LightGrid, BufferBindingIndices::LightDistances },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) });

		m_SSAOCameraSet = CreateBufferDescriptorSet(pDevice, usePushConstants, 1, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalClusterSetLayout, &m_SSAOConstantsBuffer, 0, sizeof(SSAOConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 1, { BufferBindingIndices::SampleDome },
			{ BufferType::BT_Uniform }, { STF_Fragment }, { m_SamplePointsDomeSSBO },
			{ {0, sizeof(glm::vec3)*m_SSAOKernelSize} }, m_GlobalSampleDomeSetLayout, m_GlobalSampleDomeSet);

		m_SSAOSamplersSetLayout = CreateSamplerDescriptorLayout(pDevice, 2, { 0, 1 }, { STF_Fragment }, { "Normal", "Depth" });
		m_NoiseSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 2 }, { STF_Fragment }, { "Noise" });
		m_CameraSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 6, { 0, 1, 2, 3, 4, 5 }, { STF_Fragment }, { "Color", "Normal", "Depth" });
		m_NoiseSamplerSet = CreateSamplerDescriptorSet(pDevice, 1, { m_SampleNoiseTexture }, m_NoiseSamplerSetLayout);
		m_DisplayCopySamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "Color" });
		m_ShadowAtlasSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "ShadowAtlas" });
		m_ObjectDataSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::WorldTransforms }, { BT_Storage }, { STF_Vertex });
		m_LightDistancesSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::LightDistances }, { BT_Storage }, { STF_Compute });

		m_PickingResultSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::PickingResults }, { BT_Storage }, { STF_Compute });
		m_PickingSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 3, { 0, 1, 2 }, { STF_Compute }, { "ObjectID", "Normal", "Depth" });

		ResetLightDistances = new uint32_t[MAX_LIGHTS];
		for (size_t i = 0; i < MAX_LIGHTS; ++i)
			ResetLightDistances[i] = NUM_DEPTH_SLICES;

		assert(m_ImageCount > 0);
		m_ShadowsPasses.resize(m_ImageCount);
		m_ShadowAtlasses.resize(m_ImageCount);
		m_ShadowAtlasSamplerSets.resize(m_ImageCount);
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		for (size_t i = 0; i < m_ShadowsPasses.size(); ++i)
		{
			RenderPassInfo shadowsPassInfo;
			shadowsPassInfo.RenderTextureInfo.EnableDepthStencilSampling = true;
			shadowsPassInfo.RenderTextureInfo.HasDepth = true;
			shadowsPassInfo.RenderTextureInfo.HasStencil = false;
			shadowsPassInfo.RenderTextureInfo.Width = 4096;
			shadowsPassInfo.RenderTextureInfo.Height = 4096;

			m_ShadowsPasses[i] = pDevice->CreateRenderPass(std::move(shadowsPassInfo));
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_ShadowsPasses[i]);
			TextureHandle texture = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			TextureCreateInfo info;
			info.m_Width = 4096;
			info.m_Height = 4096;
			m_ShadowAtlasses[i] = CreateGPUTextureAtlas(std::move(info), texture);
			m_ShadowAtlasSamplerSets[i] = CreateSamplerDescriptorSet(pDevice, 1, { texture }, m_ShadowAtlasSamplerSetLayout);
		}

		m_FrameCommandBuffers.resize(m_ImageCount, 0ull);

		m_FinalFrameColorPasses.resize(m_ImageCount);
		m_FinalFrameColorSets.resize(m_ImageCount);
		for (size_t i = 0; i < m_FinalFrameColorPasses.size(); ++i)
		{
			RenderPassInfo info;
			info.RenderTextureInfo.HasDepth = false;
			info.RenderTextureInfo.HasStencil = false;
			info.RenderTextureInfo.EnableDepthStencilSampling = false;
			info.RenderTextureInfo.Width = m_Resolution.x;
			info.RenderTextureInfo.Height = m_Resolution.y;
			info.RenderTextureInfo.Attachments.push_back(
				Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Unorm,
					Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float)
			);
			m_FinalFrameColorPasses[i] = pDevice->CreateRenderPass(std::move(info));

			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[i]);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			DescriptorSetInfo dsInfo;
			dsInfo.m_Layout = m_DisplayCopySamplerSetLayout;
			dsInfo.m_Samplers.resize(1);
			dsInfo.m_Samplers[0].m_TextureHandle = color;
			m_FinalFrameColorSets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
		}

		m_LightDistancesSSBOs.resize(m_ImageCount, 0ull);
		m_LightDistancesSets.resize(m_ImageCount, 0ull);
		for (size_t i = 0; i < m_LightDistancesSSBOs.size(); ++i)
		{
			if (!m_LightDistancesSSBOs[i])
				m_LightDistancesSSBOs[i] = pDevice->CreateBuffer(sizeof(uint32_t)*MAX_LIGHTS, BufferType::BT_Storage, BF_ReadAndWrite);

			if (!m_LightDistancesSets[i])
			{
				DescriptorSetInfo dsInfo;
				dsInfo.m_Layout = m_LightDistancesSetLayout;
				dsInfo.m_Buffers.resize(1);
				dsInfo.m_Buffers[0].m_BufferHandle = m_LightDistancesSSBOs[i];
				dsInfo.m_Buffers[0].m_Offset = 0;
				dsInfo.m_Buffers[0].m_Size = sizeof(uint32_t)*MAX_LIGHTS;
				m_LightDistancesSets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
			}
		}
		m_ClosestLightDepthSlices.resize(MAX_LIGHTS, NUM_DEPTH_SLICES);
		GenerateShadowLODDivisions(m_MaxShadowLODs);
		GenerateShadowMapLODResolutions();

		m_LineBuffers.resize(m_ImageCount, 0ull);
		m_LineMeshes.resize(m_ImageCount, 0ull);
	}

	void GloryRendererModule::Update()
	{
		ModuleSettings& settings = Settings();
		if (!settings.IsDirty()) return;

		const UUID linesPipeline = settings.Value<uint64_t>("Lines Pipeline");
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
		const UUID pickingPipeline = settings.Value<uint64_t>("Picking");

		settings.SetDirty(false);
	}

	void GloryRendererModule::Draw()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw" };
		if (!m_Enabled) return;

		const ModuleSettings& settings = Settings();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();

		//m_pEngine->GetDebug().SubmitLines(this, &m_pEngine->Time());

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;
		const bool usePushConstants = pDevice->IsSupported(APIFeatures::PushConstants);

		for (;;)
		{
			if (!m_FrameCommandBuffers[m_CurrentFrameIndex]) break;
			const GraphicsDevice::WaitResult result = pDevice->Wait(m_FrameCommandBuffers[m_CurrentFrameIndex], 1);
			if (result == GraphicsDevice::WR_Success)
			{
				pDevice->Release(m_FrameCommandBuffers[m_CurrentFrameIndex]);
				m_FrameCommandBuffers[m_CurrentFrameIndex] = 0;

				/* Read buffers since they are guaranteed to be finished writing to */
				const BufferHandle lightDistancesSSBO = m_LightDistancesSSBOs[m_CurrentFrameIndex];
				pDevice->ReadBuffer(lightDistancesSSBO, m_ClosestLightDepthSlices.data(), 0, m_FrameData.ActiveLights.count()*sizeof(uint32_t));

				/* @todo: Reset using a compute shader */
				pDevice->AssignBuffer(lightDistancesSSBO, ResetLightDistances, 0, m_FrameData.ActiveLights.count()*sizeof(uint32_t));

				/* Read picking results */
				m_PickResults.clear();
				for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
				{
					CameraRef camera = m_ActiveCameras[i];
					const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
					BufferHandle pickResults = uniqueCameraData.m_PickResultsSSBOs[m_CurrentFrameIndex];
					uint32_t numPicks = 0;
					pDevice->ReadBuffer(pickResults, &numPicks, 0, sizeof(uint32_t));
					if (numPicks == 0) continue;

					std::vector<GPUPickResult> results(numPicks);
					pDevice->ReadBuffer(pickResults, results.data(), sizeof(uint32_t)*4, numPicks*sizeof(GPUPickResult));
					const size_t start = m_PickResults.size();
					m_PickResults.reserve(m_PickResults.size() + numPicks);

					for (size_t j = 0; j < results.size(); ++j)
					{
						const GPUPickResult& pick = results[j];
						if (!pick.SceneID || !pick.ObjectID) continue;
						m_PickResults.emplace_back(PickResult{ camera.GetUUID(),
							SceneObjectRef{ pick.SceneID, pick.ObjectID }, glm::vec3(pick.Position), glm::vec3(pick.Normal) });
					}
				}

				break;
			}
			if (result == GraphicsDevice::WR_Timeout) continue;
			m_pEngine->GetDebug().LogError("Failed to wait for render finished!");
		}

		std::vector<SemaphoreHandle> waitSemaphores;
		std::vector<SemaphoreHandle> signalSemaphores;
		if (m_Swapchain)
		{
			pDevice->AqcuireNextSwapchainImage(m_Swapchain, &m_CurrentFrameIndex, m_ImageAvailableSemaphores[m_CurrentSemaphoreIndex]);
			waitSemaphores.push_back(m_ImageAvailableSemaphores[m_CurrentSemaphoreIndex]);
			signalSemaphores.push_back(m_RenderingFinishedSemaphores[m_CurrentSemaphoreIndex]);
		}

		PrepareDataPass();

		/* Make sure every camera has a render pass */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];
			UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			const auto& resolution = camera.GetResolution();

			if (!m_SSAOPipeline)
			{
				const UUID ssaoPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
				PipelineData* pPipeline = pipelines.GetPipelineData(ssaoPipeline);
				m_SSAOPipeline = pDevice->CreatePipeline(uniqueCameraData.m_SSAORenderPasses[0], pPipeline,
					{ m_GlobalClusterSetLayout, m_GlobalSampleDomeSetLayout, m_SSAOSamplersSetLayout, m_NoiseSamplerSetLayout },
					sizeof(glm::vec3), { AttributeType::Float3 });
			}
			if (!m_LineRenderPipeline)
			{
				const UUID lineRenderPipeline = settings.Value<uint64_t>("Lines Pipeline");
				PipelineData* pPipeline = pipelines.GetPipelineData(lineRenderPipeline);
				m_LineRenderPipeline = pDevice->CreatePipeline(uniqueCameraData.m_RenderPasses[0], pPipeline,
					{ m_GlobalLineRenderSetLayout }, sizeof(LineVertex), { AttributeType::Float3, AttributeType::Float4 });
			}

			BufferHandle& clusterSSBOHandle = uniqueCameraData.m_ClusterSSBO;
			if (!uniqueCameraData.m_ClusterSSBO)
			{
				uniqueCameraData.m_ClusterSSBO = pDevice->CreateBuffer(sizeof(VolumeTileAABB)*NUM_CLUSTERS, BufferType::BT_Storage, BF_CopyDst);
				DescriptorSetInfo setInfo;
				setInfo.m_Layout = m_CameraClusterSetLayout;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = clusterSSBOHandle;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(VolumeTileAABB)*NUM_CLUSTERS;
				uniqueCameraData.m_ClusterSet = pDevice->CreateDescriptorSet(std::move(setInfo));

				GenerateClusterSSBO(i, pDevice, camera, uniqueCameraData.m_ClusterSet);

				auto iter = std::find(m_DirtyCameraPerspectives.begin(), m_DirtyCameraPerspectives.end(), camera);
				if (iter != m_DirtyCameraPerspectives.end())
					m_DirtyCameraPerspectives.erase(iter);
			}
		}

		/* Command buffer start */
		m_FrameCommandBuffers[m_CurrentFrameIndex] = pDevice->Begin();

		/* Light cluster culling */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera " + std::to_string(i) + " light cluster culling" };
			ClusterPass(m_FrameCommandBuffers[m_CurrentFrameIndex], static_cast<uint32_t>(i));
		}

		/* Shadows */
		ShadowMapsPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);

		const GPUTextureAtlas& shadowAtlas = GetGPUTextureAtlas(m_ShadowAtlasses[m_CurrentFrameIndex]);
		/* Wait shadow rendering to finish */
		pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { shadowAtlas.GetTexture() },
			PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);

		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera " + std::to_string(i) };
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			const BufferHandle& lightIndexSSBO = uniqueCameraData.m_LightIndexSSBOs[m_CurrentFrameIndex];
			const BufferHandle& lightGridSSBO = uniqueCameraData.m_LightGridSSBOs[m_CurrentFrameIndex];

			/* Wait for light culling to finish */
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], { lightIndexSSBO, lightGridSSBO }, {},
				PipelineStageFlagBits::PST_ComputeShader, PipelineStageFlagBits::PST_FragmentShader);

			/* Draw objects */
			const RenderPassHandle& renderPass = uniqueCameraData.m_RenderPasses[m_CurrentFrameIndex];
			pDevice->SetRenderPassClear(renderPass, camera.GetClearColor());
			pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], renderPass);
			DynamicObjectsPass(m_FrameCommandBuffers[m_CurrentFrameIndex], static_cast<uint32_t>(i));

			if (m_LineVertexCount)
			{
				uint32_t cameraIndex = static_cast<uint32_t>(i);
				pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineRenderPipeline);
				pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineRenderPipeline, { m_GlobalLineRenderSet });
				if (usePushConstants)
					pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineRenderPipeline, 0, sizeof(uint32_t), &cameraIndex, STF_Vertex);
				else
					pDevice->AssignBuffer(m_LineRenderConstantsBuffer, &cameraIndex, sizeof(uint32_t));
				pDevice->DrawMesh(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineMeshes[m_CurrentFrameIndex]);
				pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			}
			pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}

		/* Picking */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera " + std::to_string(i) + " late" };
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			if (uniqueCameraData.m_Picks.empty()) continue;

			const RenderPassHandle& renderPass = uniqueCameraData.m_RenderPasses[m_CurrentFrameIndex];
			const RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			const TextureHandle objectID = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			const TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 2);
			const TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 3);

			/* Wait for rendering to finish */
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { objectID, normals, depth },
				PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_ComputeShader);

			const DescriptorSetHandle pickingResultSet = uniqueCameraData.m_PickingResultSets[m_CurrentFrameIndex];
			const DescriptorSetHandle pickingSamplersSet = uniqueCameraData.m_PickingSamplersSets[m_CurrentFrameIndex];

			const size_t count = std::min(uniqueCameraData.m_Picks.size(), MaxPicks);

			PickingConstants constants;
			constants.m_CameraIndex = i;
			constants.m_NumPickings = count;
			std::memcpy(constants.m_Picks, uniqueCameraData.m_Picks.data(), count*sizeof(glm::ivec2));

			pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_PickingPipeline);
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_PickingPipeline,
				{ m_GlobalPickingSet, pickingResultSet, pickingSamplersSet });
			if (usePushConstants)
				pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], m_PickingPipeline,
					0, sizeof(PickingConstants), &constants, STF_Compute);
			else
				pDevice->AssignBuffer(m_PickingConstantsBuffer, &constants, sizeof(PickingConstants));
			pDevice->Dispatch(m_FrameCommandBuffers[m_CurrentFrameIndex], 1, 1, 1);
			pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}

		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera " + std::to_string(i) + " late"};
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			const BufferHandle pickResults = uniqueCameraData.m_PickResultsSSBOs[m_CurrentFrameIndex];

			/* Wait for picking to finish */
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], { pickResults }, {},
				PipelineStageFlagBits::PST_ComputeShader, PipelineStageFlagBits::PST_FragmentShader);

			/* Draw late objects */
			const RenderPassHandle& renderPass = uniqueCameraData.m_LateRenderPasses[m_CurrentFrameIndex];
			pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], renderPass);
			DynamicLateObjectsPass(m_FrameCommandBuffers[m_CurrentFrameIndex], static_cast<uint32_t>(i));
			pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}

		/* SSAO pass */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera " + std::to_string(i) + " SSAO pass" };
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());

			const RenderPassHandle& renderPass = uniqueCameraData.m_RenderPasses[m_CurrentFrameIndex];
			const RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			const RenderPassHandle& ssaoRenderPass = uniqueCameraData.m_SSAORenderPasses[m_CurrentFrameIndex];
			const DescriptorSetHandle& ssaoSamplersSet = uniqueCameraData.m_SSAOSamplersSets[m_CurrentFrameIndex];

			SSAOConstants constants;
			constants.CameraIndex = i;
			constants.KernelSize = m_GlobalSSAOSetting.m_KernelSize;
			constants.SampleRadius = m_GlobalSSAOSetting.m_SampleRadius;
			constants.SampleBias = m_GlobalSSAOSetting.m_SampleBias;

			const glm::uvec2& resolution = camera.GetResolution();

			const TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 1);
			const TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 2);
			const TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 3);

			/* Wait for rendering to finish */
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { normals, depth },
				PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);

			/* SSAO pass */
			pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], ssaoRenderPass);
			pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SSAOPipeline);
			pDevice->SetViewport(m_FrameCommandBuffers[m_CurrentFrameIndex], 0.0f, 0.0f, float(resolution.x), float(resolution.y));
			pDevice->SetScissor(m_FrameCommandBuffers[m_CurrentFrameIndex], 0, 0, resolution.x, resolution.y);
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SSAOPipeline, { m_SSAOCameraSet, m_GlobalSampleDomeSet, ssaoSamplersSet, m_NoiseSamplerSet });
			if (!m_SSAOConstantsBuffer)
				pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SSAOPipeline, 0, sizeof(SSAOConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
			else
				pDevice->AssignBuffer(m_SSAOConstantsBuffer, &constants, sizeof(SSAOConstants));
			pDevice->DrawQuad(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}

		std::vector<TextureHandle> colorTextures(m_OutputCameras.size());
		for (size_t i = 0; i < m_OutputCameras.size(); ++i)
		{
			CameraRef camera = m_OutputCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(uniqueCameraData.m_RenderPasses[m_CurrentFrameIndex]);
			colorTextures[i] = pDevice->GetRenderTextureAttachment(renderTexture, 1);
		}

		if (!colorTextures.empty())
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, colorTextures,
				PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);

		pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], m_FinalFrameColorPasses[m_CurrentFrameIndex]);
		pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_DisplayCopyPipeline);
		for (size_t i = 0; i < m_OutputCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Render output from camera " + std::to_string(i) };
			CameraRef camera = m_OutputCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_DisplayCopyPipeline, { uniqueCameraData.m_FinalColorSamplerSets[m_CurrentFrameIndex] });
			pDevice->DrawQuad(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}
		pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);

		if (m_Swapchain)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Render to swap chain image" };
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[m_CurrentFrameIndex]);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 0);

			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { color },
				PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);

			pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SwapchainPasses[m_CurrentSemaphoreIndex]);
			pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_DisplayCopyPipeline);
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_DisplayCopyPipeline, { m_FinalFrameColorSets[m_CurrentFrameIndex] });
			pDevice->DrawQuad(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}
		pDevice->End(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		pDevice->Commit(m_FrameCommandBuffers[m_CurrentFrameIndex], waitSemaphores, signalSemaphores);
	}

	void GloryRendererModule::Cleanup()
	{
	}

	void GloryRendererModule::LoadSettings(ModuleSettings& settings)
	{
		RendererModule::LoadSettings(settings);
		settings.RegisterAssetReference<PipelineData>("Lines Pipeline", 19);
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
		settings.RegisterAssetReference<PipelineData>("Picking", 47);
	}

	size_t GloryRendererModule::DefaultAttachmenmtIndex() const
	{
		return 4;
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
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		RenderPassHandle renderPass = uniqueCameraData.m_RenderPasses[m_CurrentFrameIndex];
		RenderPassHandle ssaoRenderPass = uniqueCameraData.m_SSAORenderPasses[m_CurrentFrameIndex];
		if (!renderPass || !ssaoRenderPass) return NULL;
		if (index == 4)
		{
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			return pDevice->GetRenderTextureAttachment(renderTexture, 1);
		}
		if (index >= 3)
		{
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(ssaoRenderPass);
			return pDevice->GetRenderTextureAttachment(renderTexture, index-3);
		}

		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
		return pDevice->GetRenderTextureAttachment(renderTexture, index);
	}

	TextureHandle GloryRendererModule::FinalColor() const
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[m_CurrentFrameIndex]);
		return pDevice->GetRenderTextureAttachment(renderTexture, 0);
	}

	size_t GloryRendererModule::DebugOverlayCount() const
	{
		return DebugOverlayNameCount;
	}

	std::string_view GloryRendererModule::DebugOverlayName(size_t index) const
	{
		return DebugOverlayNames[index];
	}

	TextureHandle GloryRendererModule::DebugOverlay(size_t index) const
	{
		switch (index == 0)
		{
		default:
			return GetGPUTextureAtlas(m_ShadowAtlasses[m_CurrentFrameIndex]).GetTexture();
		}
		return NULL;
	}

	void GloryRendererModule::OnWindowResized()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::OnWindowResized" };
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;
		pDevice->WaitIdle();

		for (size_t i = 0; i < m_FinalFrameColorPasses.size(); ++i)
		{
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[i]);
			pDevice->ResizeRenderTexture(renderTexture, m_Resolution.x, m_Resolution.y);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 0);

			DescriptorSetUpdateInfo updateInfo;
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = color;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(m_FinalFrameColorSets[i], updateInfo);
		}

		if (!m_Swapchain) return;

		for (size_t i = 0; i < m_SwapchainPasses.size(); ++i)
			pDevice->FreeRenderPass(m_SwapchainPasses[i]);

		pDevice->RecreateSwapchain(m_Swapchain);
		OnSwapchainChanged();
	}

	void GloryRendererModule::OnSwapchainChanged()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::OnSwapchainChanged" };
		if (!m_Swapchain) return;
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;
		m_ImageCount = pDevice->GetSwapchainImageCount(m_Swapchain);
		assert(m_ImageCount > 0);
		m_SwapchainPasses.resize(m_ImageCount, 0ull);
		m_RenderingFinishedSemaphores.resize(m_ImageCount, 0ull);
		m_ImageAvailableSemaphores.resize(m_ImageCount, 0ull);
		m_FrameCommandBuffers.resize(m_ImageCount, 0ull);
		m_ShadowsPasses.resize(m_ImageCount, 0ull);
		m_ShadowAtlasses.resize(m_ImageCount, 0ull);
		m_ShadowAtlasSamplerSets.resize(m_ImageCount, 0ull);
		m_FinalFrameColorPasses.resize(m_ImageCount, 0ull);
		m_FinalFrameColorSets.resize(m_ImageCount, 0ull);
		m_LightDistancesSSBOs.resize(m_ImageCount, 0ull);
		m_LightDistancesSets.resize(m_ImageCount, 0ull);
		m_LineBuffers.resize(m_ImageCount, 0ull);
		m_LineMeshes.resize(m_ImageCount, 0ull);

		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		const ModuleSettings& settings = Settings();
		for (size_t i = 0; i < m_ImageCount; ++i)
		{
			TextureHandle image = pDevice->GetSwapchainImage(m_Swapchain, i);
			RenderPassInfo renderPassInfo;
			renderPassInfo.RenderTextureInfo.HasDepth = false;
			renderPassInfo.RenderTextureInfo.HasStencil = false;
			renderPassInfo.RenderTextureInfo.EnableDepthStencilSampling = false;
			renderPassInfo.RenderTextureInfo.Width = m_Resolution.x;
			renderPassInfo.RenderTextureInfo.Height = m_Resolution.y;
			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("color", PixelFormat::PF_BGRA, PixelFormat::PF_B8G8R8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float, false));
			renderPassInfo.RenderTextureInfo.Attachments[0].Texture = image;
			if (!m_SwapchainPasses[i])
				m_SwapchainPasses[i] = pDevice->CreateRenderPass(std::move(renderPassInfo));
			if (!m_RenderingFinishedSemaphores[i])
				m_RenderingFinishedSemaphores[i] = pDevice->CreateSemaphore();
			if (!m_ImageAvailableSemaphores[i])
				m_ImageAvailableSemaphores[i] = pDevice->CreateSemaphore();

			RenderPassInfo shadowsPassInfo;
			shadowsPassInfo.RenderTextureInfo.EnableDepthStencilSampling = true;
			shadowsPassInfo.RenderTextureInfo.HasDepth = true;
			shadowsPassInfo.RenderTextureInfo.HasStencil = false;
			shadowsPassInfo.RenderTextureInfo.Width = 4096;
			shadowsPassInfo.RenderTextureInfo.Height = 4096;
			if (!m_ShadowsPasses[i])
				m_ShadowsPasses[i] = pDevice->CreateRenderPass(std::move(shadowsPassInfo));

			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_ShadowsPasses[i]);
			TextureHandle texture = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			TextureCreateInfo info;
			info.m_Width = 4096;
			info.m_Height = 4096;
			if (!m_ShadowAtlasses[i])
			{
				m_ShadowAtlasses[i] = CreateGPUTextureAtlas(std::move(info), texture);
				m_ShadowAtlasSamplerSets[i] = CreateSamplerDescriptorSet(pDevice, 2, { NULL, texture }, m_ShadowAtlasSamplerSetLayout);
			}

			RenderPassInfo finalColorPassInfo;
			finalColorPassInfo.RenderTextureInfo.HasDepth = false;
			finalColorPassInfo.RenderTextureInfo.HasStencil = false;
			finalColorPassInfo.RenderTextureInfo.EnableDepthStencilSampling = false;
			finalColorPassInfo.RenderTextureInfo.Width = m_Resolution.x;
			finalColorPassInfo.RenderTextureInfo.Height = m_Resolution.y;
			finalColorPassInfo.RenderTextureInfo.Attachments.push_back(
				Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Unorm,
					Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float)
			);
			if (!m_FinalFrameColorPasses[i])
				m_FinalFrameColorPasses[i] = pDevice->CreateRenderPass(std::move(finalColorPassInfo));

			if (!m_FinalFrameColorSets[i])
			{
				RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[i]);
				TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 0);
				DescriptorSetInfo dsInfo;
				dsInfo.m_Layout = m_DisplayCopySamplerSetLayout;
				dsInfo.m_Samplers.resize(1);
				dsInfo.m_Samplers[0].m_TextureHandle = color;
				m_FinalFrameColorSets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
			}

			if (!m_LightDistancesSSBOs[i])
				m_LightDistancesSSBOs[i] = pDevice->CreateBuffer(sizeof(uint32_t)*MAX_LIGHTS, BufferType::BT_Storage, BF_ReadAndWrite);

			if (!m_LightDistancesSets[i])
			{
				DescriptorSetInfo dsInfo;
				dsInfo.m_Layout = m_LightDistancesSetLayout;
				dsInfo.m_Buffers.resize(1);
				dsInfo.m_Buffers[0].m_BufferHandle = m_LightDistancesSSBOs[i];
				dsInfo.m_Buffers[0].m_Offset = 0;
				dsInfo.m_Buffers[0].m_Size = sizeof(uint32_t)*MAX_LIGHTS;
				m_LightDistancesSets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
			}
		}

		m_CurrentSemaphoreIndex = 0;
		m_CurrentFrameIndex = 0;
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

	void GloryRendererModule::RenderBatches(CommandBufferHandle commandBuffer, const std::vector<PipelineBatch>& batches,
		const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex, DescriptorSetHandle globalRenderSet, const glm::vec4& viewport,
		DescriptorSetHandle shadowsSet)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::RenderBatches" };
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		MaterialManager& materialManager = m_pEngine->GetMaterialManager();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		AssetManager& assets = m_pEngine->GetAssetManager();

		const bool usePushConstants = pDevice->IsSupported(APIFeatures::PushConstants);

		RenderConstants constants;
		constants.m_CameraIndex = static_cast<uint32_t>(cameraIndex);
		constants.m_LightCount = m_FrameData.ActiveLights.count();
		constants.m_GridSize = glm::uvec4(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES, 0.0f);
		CameraRef camera = m_ActiveCameras[cameraIndex];
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		const DescriptorSetHandle lightSet = uniqueCameraData.m_LightSets[m_CurrentFrameIndex];
		const LayerMask& cameraMask = camera.GetLayerMask();
		if (!usePushConstants) pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));

		size_t batchIndex = 0;
		for (const PipelineBatch& pipelineRenderData : batches)
		{
			if (batchIndex >= batchDatas.size()) break;
			const PipelineBatchData& batchData = batchDatas.at(batchIndex);
			++batchIndex;

			PipelineData* pPipelineData = pipelines.GetPipelineData(pipelineRenderData.m_PipelineID);
			if (!pPipelineData) continue;
			pDevice->BeginPipeline(commandBuffer, batchData.m_Pipeline);
			if (viewport.z > 0.0f && viewport.w > 0.0f)
			{
				pDevice->SetViewport(commandBuffer, int(viewport.x), int(viewport.y), viewport.z, viewport.w);
				pDevice->SetScissor(commandBuffer, int(viewport.x), int(viewport.y), viewport.z, viewport.w);
			}

			pDevice->BindDescriptorSets(commandBuffer, batchData.m_Pipeline,
				{ globalRenderSet, batchData.m_ObjectDataSet, m_GlobalLightSet, lightSet, batchData.m_MaterialSet });

			if (shadowsSet)
				pDevice->BindDescriptorSets(commandBuffer, batchData.m_Pipeline, { shadowsSet }, 5);

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

					if (cameraMask != 0 && meshBatch.m_LayerMasks[i] != 0 &&
						(cameraMask & meshBatch.m_LayerMasks[i]) == 0) continue;

					const UUID materialID = pipelineRenderData.m_UniqueMaterials[meshBatch.m_MaterialIndices[i]];
					MaterialData* pMaterialData = materialManager.GetMaterial(materialID);
					if (!pMaterialData) continue;

					const auto& ids = meshBatch.m_ObjectIDs[i];
					constants.m_ObjectID = ids.second;
					constants.m_SceneID = ids.first;
					constants.m_ObjectDataIndex = currentObject;
					constants.m_MaterialIndex = meshBatch.m_MaterialIndices[i];

					if (usePushConstants)
						pDevice->PushConstants(commandBuffer, batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
					else
						pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));
					if (!batchData.m_TextureSets.empty())
						pDevice->BindDescriptorSets(commandBuffer, batchData.m_Pipeline, { batchData.m_TextureSets[constants.m_MaterialIndex] }, 6);
					pDevice->DrawMesh(commandBuffer, mesh);
				}
			}

			pDevice->EndPipeline(commandBuffer);
		}
	}

	void GloryRendererModule::PrepareDataPass()
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::PrepareDataPass" };
		const ModuleSettings& settings = Settings();
		const UUID clusterGeneratorPipeline = settings.Value<uint64_t>("Cluster Generator");
		const UUID clusterCullLightPipeline = settings.Value<uint64_t>("Cluster Cull Light");
		const UUID pickingPipeline = settings.Value<uint64_t>("Picking");
		const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		if (!m_ClusterGeneratorPipeline)
		{
			PipelineData* pPipeline = pipelines.GetPipelineData(clusterGeneratorPipeline);
			m_ClusterGeneratorPipeline = pDevice->CreateComputePipeline(pPipeline, { m_GlobalClusterSetLayout, m_CameraClusterSetLayout });
		}
		if (!m_ClusterCullLightPipeline)
		{
			PipelineData* pPipeline = pipelines.GetPipelineData(clusterCullLightPipeline);
			m_ClusterCullLightPipeline = pDevice->CreateComputePipeline(pPipeline,
				{ m_GlobalClusterSetLayout, m_CameraClusterSetLayout, m_GlobalLightSetLayout, m_CameraLightSetLayout, m_LightDistancesSetLayout });
		}
		if (!m_PickingPipeline)
		{
			PipelineData* pPipeline = pipelines.GetPipelineData(pickingPipeline);
			m_PickingPipeline = pDevice->CreateComputePipeline(pPipeline,
				{ m_GlobalPickingSetLayout, m_PickingResultSetLayout, m_PickingSamplerSetLayout });
		}
		if (!m_DisplayCopyPipeline)
		{
			PipelineData* pPipeline = pipelines.GetPipelineData(displayPipeline);
			m_DisplayCopyPipeline = pDevice->CreatePipeline(m_FinalFrameColorPasses[0], pPipeline, { m_DisplayCopySamplerSetLayout },
				sizeof(glm::vec3), { AttributeType::Float3 });
		}
		if (!m_ShadowRenderPipeline)
		{
			const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
			PipelineData* pPipeline = pipelines.GetPipelineData(shadowsPipeline);
			pPipeline->GetCullFace() = CullFace::Front;
			m_ShadowRenderPipeline = pDevice->CreatePipeline(m_ShadowsPasses[0], pPipeline, {m_GlobalShadowRenderSetLayout, m_ObjectDataSetLayout }, sizeof(DefaultVertex3D),
				{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
				AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });
		}
		/*if (!m_TransparentShadowRenderPipeline)
		{
			const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");
			PipelineData* pPipeline = pipelines.GetPipelineData(shadowsTransparentPipeline);
			pPipeline->GetCullFace() = CullFace::Front;
			m_TransparentShadowRenderPipeline = pDevice->CreatePipeline(m_ShadowsPasses[0], pPipeline, {}, sizeof(DefaultVertex3D),
				{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
				AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });
		}*/

		PrepareCameras();

		for (size_t i = 0; i < m_FrameData.Picking.size(); ++i)
		{
			const glm::ivec2& pos = m_FrameData.Picking[i].first;
			const UUID cameraID = m_FrameData.Picking[i].second;
			UniqueCameraData& cameraData = m_UniqueCameraDatas.at(cameraID);
			cameraData.m_Picks.emplace_back(pos);
		}

		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			if (std::find(m_DirtyCameraPerspectives.begin(), m_DirtyCameraPerspectives.end(), camera) != m_DirtyCameraPerspectives.end())
				GenerateClusterSSBO(i, pDevice, camera, uniqueCameraData.m_ClusterSet);
		}
		m_DirtyCameraPerspectives.clear();

		if (m_LightCameraDatas->size() < m_FrameData.LightSpaceTransforms.count()) m_LightCameraDatas.resize(m_FrameData.LightSpaceTransforms.count());
		for (size_t i = 0; i < m_FrameData.LightSpaceTransforms.count(); ++i)
		{
			m_LightCameraDatas.m_Data[i].m_Projection = glm::identity<glm::mat4>();
			if (m_LightCameraDatas.m_Data[i].m_View != m_FrameData.LightSpaceTransforms[i])
			{
				m_LightCameraDatas.m_Data[i].m_View = m_FrameData.LightSpaceTransforms[i];
				m_LightCameraDatas.SetDirty(i);
			}
		}
		if (m_LightCameraDatas)
		{
			const size_t dirtySize = m_LightCameraDatas.DirtySize();
			pDevice->AssignBuffer(m_LightCameraDatasBuffer, m_LightCameraDatas.DirtyStart(),
				m_LightCameraDatas.m_DirtyRange.first*sizeof(PerCameraData), dirtySize*sizeof(PerCameraData));
		}

		/* Prepare shadow resolutions and atlas coords */
		const uint32_t sliceSteps = NUM_DEPTH_SLICES/m_MaxShadowLODs;

		GPUTextureAtlas& shadowAtlas = GetGPUTextureAtlas(m_ShadowAtlasses[m_CurrentFrameIndex]);
		shadowAtlas.ReleaseAllChunks();
		for (size_t i = 0; i < m_FrameData.ActiveLights.count(); ++i)
		{
			auto& lightData = m_FrameData.ActiveLights[i];
			const auto& lightTransform = m_FrameData.LightSpaceTransforms[i];
			const auto& lightID = m_FrameData.ActiveLightIDs[i];

			if (!lightData.shadowsEnabled) continue;

			const uint32_t depthSlice = m_ClosestLightDepthSlices[i];
			/* No need to render that which can't be seen! */
			if (depthSlice == NUM_DEPTH_SLICES)
			{
				lightData.shadowsEnabled = 0;
				continue;
			}

			const uint32_t shadowLOD = std::min(depthSlice/sliceSteps, uint32_t(m_MaxShadowLODs - 1));
			const glm::uvec2 shadowMapResolution = m_ShadowMapResolutions[shadowLOD];

			const UUID chunkID = shadowAtlas.ReserveChunk(shadowMapResolution.x, shadowMapResolution.y, lightID);
			if (!chunkID)
			{
				lightData.shadowsEnabled = 0;
				m_pEngine->GetDebug().LogError("Failed to reserve chunk in shadow atlas, there is not enough space left.");
				continue;
			}
			lightData.shadowCoords = shadowAtlas.GetChunkCoords(lightID);
		}

		/* Update light data */
		pDevice->AssignBuffer(m_LightsSSBO, m_FrameData.ActiveLights.data(), 0, MAX_LIGHTS*sizeof(LightData));
		pDevice->AssignBuffer(m_LightSpaceTransformsSSBO, m_FrameData.LightSpaceTransforms.data(), 0, MAX_LIGHTS*sizeof(glm::mat4));

		PrepareBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData);
		PrepareBatches(m_DynamicLatePipelineRenderDatas, m_DynamicLateBatchData);
		PrepareLineMesh(pDevice);
	}

	void GloryRendererModule::PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::PrepareBatches" };
		if (m_ActiveCameras.empty()) return;
		CameraRef defaultCamera = m_ActiveCameras[0];
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(defaultCamera.GetUUID());
		const RenderPassHandle& defaultRenderPass = uniqueCameraData.m_RenderPasses[0];
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
					batchData.m_Worlds.SetDirty(meshIndex);
				}
				meshIndex += meshBatch.m_Worlds.size();
			}

			const size_t propertyDataSize = pPipelineData->TotalPropertiesByteSize();
			const size_t textureCount = pPipelineData->ResourcePropertyCount();
			const size_t paddingBytes = 32 - propertyDataSize % 32;
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
					texturesSetLayoutInfo.m_Samplers[i].m_BindingIndex = static_cast<uint32_t>(i + 1);
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
					batchData.m_MaterialDatas.SetDirty(i);
				}
				const uint32_t textureBits = pMaterialData->TextureSetBits();
				if (textureCount && batchData.m_TextureBits.m_Data[i] != textureBits)
				{
					batchData.m_TextureBits.m_Data[i] = textureBits;
					batchData.m_TextureBits.SetDirty(i);
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
				batchData.m_WorldsBuffer = pDevice->CreateBuffer(batchData.m_Worlds->size()*sizeof(glm::mat4), BT_Storage, BF_Write);
				batchData.m_Worlds.SetDirty();
			}
			if (batchData.m_Worlds)
			{
				const size_t dirtySize = batchData.m_Worlds.DirtySize();
				pDevice->AssignBuffer(batchData.m_WorldsBuffer, batchData.m_Worlds.DirtyStart(),
					batchData.m_Worlds.m_DirtyRange.first*sizeof(glm::mat4), dirtySize*sizeof(glm::mat4));
			}

			if (!batchData.m_MaterialsBuffer)
			{
				batchData.m_MaterialsBuffer = pDevice->CreateBuffer(batchData.m_MaterialDatas->size(), BT_Storage, BF_Write);
				batchData.m_MaterialDatas.SetDirty();
			}
			if (batchData.m_MaterialDatas)
			{
				const size_t dirtySize = batchData.m_MaterialDatas.DirtySize();
				pDevice->AssignBuffer(batchData.m_MaterialsBuffer, batchData.m_MaterialDatas.DirtyStart(),
					batchData.m_MaterialDatas.m_DirtyRange.first, dirtySize);
			}

			if (textureCount && !batchData.m_TextureBitsBuffer)
			{
				batchData.m_TextureBitsBuffer = pDevice->CreateBuffer(batchData.m_TextureBits->size()*sizeof(uint32_t), BT_Storage, BF_Write);
				batchData.m_TextureBits.SetDirty();
			}
			if (textureCount && batchData.m_TextureBits)
			{
				const size_t dirtySize = batchData.m_TextureBits.DirtySize();
				pDevice->AssignBuffer(batchData.m_TextureBitsBuffer, batchData.m_TextureBits.DirtyStart(),
					batchData.m_TextureBits.m_DirtyRange.first*sizeof(uint32_t), dirtySize*sizeof(uint32_t));
			}

			if (!batchData.m_ObjectDataSet)
			{
				DescriptorSetInfo setInfo;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = batchData.m_WorldsBuffer;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = batchData.m_Worlds->size() * sizeof(glm::mat4);
				setInfo.m_Layout = m_ObjectDataSetLayout;
				batchData.m_ObjectDataSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}
			if(!batchData.m_MaterialSet)
			{
				DescriptorSetLayoutInfo setLayoutInfo;
				DescriptorSetInfo setInfo;
				setInfo.m_Buffers.resize(textureCount > 0 ? 2 : 1);
				setLayoutInfo.m_Buffers.resize(textureCount > 0 ? 2 : 1);
				setLayoutInfo.m_Buffers[0].m_BindingIndex = uint32_t(BufferBindingIndices::Materials);
				setLayoutInfo.m_Buffers[0].m_Type = BT_Storage;
				setLayoutInfo.m_Buffers[0].m_ShaderStages = STF_Fragment;
				setInfo.m_Buffers[0].m_BufferHandle = batchData.m_MaterialsBuffer;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = batchData.m_MaterialDatas->size();
				if (batchData.m_TextureBitsBuffer)
				{
					setLayoutInfo.m_Buffers[1].m_BindingIndex = uint32_t(BufferBindingIndices::HasTexture);
					setLayoutInfo.m_Buffers[1].m_Type = BT_Storage;
					setLayoutInfo.m_Buffers[1].m_ShaderStages = STF_Fragment;
					setInfo.m_Buffers[1].m_BufferHandle = batchData.m_TextureBitsBuffer;
					setInfo.m_Buffers[1].m_Offset = 0;
					setInfo.m_Buffers[1].m_Size = batchData.m_TextureBits->size()*sizeof(uint32_t);
				}

				batchData.m_MaterialSetLayout = setInfo.m_Layout = pDevice->CreateDescriptorSetLayout(std::move(setLayoutInfo));
				batchData.m_MaterialSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!batchData.m_Pipeline)
			{
				std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts(textureCount ? 7 : 6);
				descriptorSetLayouts[0] = m_GlobalRenderSetLayout;
				descriptorSetLayouts[1] = m_ObjectDataSetLayout;
				descriptorSetLayouts[2] = m_GlobalLightSetLayout;
				descriptorSetLayouts[3] = m_CameraLightSetLayout;
				descriptorSetLayouts[4] = batchData.m_MaterialSetLayout;
				descriptorSetLayouts[5] = m_ShadowAtlasSamplerSetLayout;
				if (textureCount) descriptorSetLayouts[6] = batchData.m_TextureSetLayout;

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

	void GloryRendererModule::GenerateClusterSSBO(uint32_t cameraIndex, GraphicsDevice* pDevice, CameraRef camera, DescriptorSetHandle clusterSet)
	{
		PrepareCameras();

		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::GenerateClusterSSBO" };
		const glm::uvec2 resolution = camera.GetResolution();

		ClusterConstants constants;
		constants.CameraIndex = cameraIndex;
		constants.LightCount = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		constants.GridSize = glm::vec4(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES, 0.0f);

		CommandBufferHandle commandBuffer = pDevice->Begin();
		pDevice->BeginPipeline(commandBuffer, m_ClusterGeneratorPipeline);
		pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
		pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
		pDevice->BindDescriptorSets(commandBuffer, m_ClusterGeneratorPipeline, { m_GlobalClusterSet, clusterSet });
		if (!m_ClusterConstantsBuffer)
			pDevice->PushConstants(commandBuffer, m_ClusterGeneratorPipeline, 0, sizeof(ClusterConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
		else
			pDevice->AssignBuffer(m_ClusterConstantsBuffer, &constants, sizeof(ClusterConstants));
		pDevice->Dispatch(commandBuffer, constants.GridSize.x, constants.GridSize.y, constants.GridSize.z);
		pDevice->EndPipeline(commandBuffer);
		pDevice->End(commandBuffer);
		pDevice->Commit(commandBuffer);
		pDevice->Wait(commandBuffer);
		pDevice->Release(commandBuffer);
	}

	void GloryRendererModule::PrepareLineMesh(GraphicsDevice* pDevice)
	{
		/* Update line buffer */
		if (m_LineVertexCount == 0) return;
		bool resizeBuffer = false;
		if (m_LineVertices->size() < m_LineVertexCount)
		{
			m_LineVertices.resize(m_LineVertexCount);
			resizeBuffer = true;
		}
		for (size_t i = 0; i < m_LineVertexCount; ++i)
		{
			if (std::memcmp(&m_pLineVertices[i], &m_LineVertices.m_Data[i], sizeof(LineVertex)) == 0)
				continue;
			m_LineVertices.m_Data[i] = m_pLineVertices[i];
			m_LineVertices.SetDirty(i);
		}

		if (m_LineBuffers[m_CurrentFrameIndex] && resizeBuffer)
			pDevice->FreeBuffer(m_LineBuffers[m_CurrentFrameIndex]);
		if (!m_LineBuffers[m_CurrentFrameIndex])
		{
			m_LineBuffers[m_CurrentFrameIndex] = pDevice->CreateBuffer(m_LineVertices->size()*sizeof(LineVertex), BT_Vertex, BF_None);
			m_LineMeshes[m_CurrentFrameIndex] = pDevice->CreateMesh({ m_LineBuffers[m_CurrentFrameIndex] }, m_LineVertexCount, 0, sizeof(LineVertex),
				PrimitiveType::PT_Lines, { AttributeType::Float3, AttributeType::Float4 });
			resizeBuffer = false;
		}
		if (m_LineVertices)
		{
			pDevice->AssignBuffer(m_LineBuffers[m_CurrentFrameIndex], m_LineVertices.m_Data.data());

			if (resizeBuffer)
				pDevice->UpdateMesh(m_LineMeshes[m_CurrentFrameIndex], { m_LineBuffers[m_CurrentFrameIndex] }, m_LineVertexCount, 0);
			else
				pDevice->UpdateMesh(m_LineMeshes[m_CurrentFrameIndex], { }, m_LineVertexCount, 0);
		}
	}

	void GloryRendererModule::ClusterPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::ClusterPass" };
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();

		CameraRef camera = m_ActiveCameras[cameraIndex];
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		const DescriptorSetHandle& clusterSet = uniqueCameraData.m_ClusterSet;
		const DescriptorSetHandle& lightSet = uniqueCameraData.m_LightSets[m_CurrentFrameIndex];
		const DescriptorSetHandle& lightDistancesSet = m_LightDistancesSets[m_CurrentFrameIndex];

		const glm::uvec2 resolution = camera.GetResolution();

		ClusterConstants constants;
		constants.CameraIndex = cameraIndex;
		constants.LightCount = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		constants.GridSize = glm::vec4(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES, 0.0f);

		pDevice->BeginPipeline(commandBuffer, m_ClusterCullLightPipeline);
		pDevice->BindDescriptorSets(commandBuffer, m_ClusterCullLightPipeline, { m_GlobalClusterSet, clusterSet, m_GlobalLightSet, lightSet, lightDistancesSet });
		if (!m_ClusterConstantsBuffer)
			pDevice->PushConstants(commandBuffer, m_ClusterCullLightPipeline, 0, sizeof(ClusterConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
		else
			pDevice->AssignBuffer(m_ClusterConstantsBuffer, &constants, sizeof(ClusterConstants));
		pDevice->Dispatch(commandBuffer, 1, 1, 6);
		pDevice->EndPipeline(commandBuffer);
	}

	void GloryRendererModule::DynamicObjectsPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::DynamicObjectsPass" };
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		CameraRef camera = m_ActiveCameras[cameraIndex];
		DescriptorSetHandle shadowAtlasSet = m_ShadowAtlasSamplerSets[m_CurrentFrameIndex];
		RenderBatches(commandBuffer, m_DynamicPipelineRenderDatas, m_DynamicBatchData, cameraIndex, m_GlobalRenderSet,
			{ 0.0f, 0.0f, camera.GetResolution() }, shadowAtlasSet);
	}

	void GloryRendererModule::DynamicLateObjectsPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::DynamicLateObjectsPass" };
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		CameraRef camera = m_ActiveCameras[cameraIndex];
		DescriptorSetHandle shadowAtlasSet = m_ShadowAtlasSamplerSets[m_CurrentFrameIndex];
		RenderBatches(commandBuffer, m_DynamicLatePipelineRenderDatas, m_DynamicLateBatchData, cameraIndex, m_GlobalRenderSet,
			{ 0.0f, 0.0f, camera.GetResolution() }, shadowAtlasSet);
	}

	std::uniform_real_distribution<float> RandomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine NumberGenerator;

	void GloryRendererModule::GenerateDomeSamplePointsSSBO(GraphicsDevice* pDevice, uint32_t size)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::GenerateDomeSamplePointsSSBO" };
		if (m_SSAOKernelSize == size) return;
		m_SSAOKernelSize = size;

		if (!m_SamplePointsDomeSSBO)
			m_SamplePointsDomeSSBO = pDevice->CreateBuffer(sizeof(glm::vec3)*MAX_KERNEL_SIZE, BufferType::BT_Uniform, BF_CopyDst);

		std::vector<glm::vec3> samplePoints{ m_SSAOKernelSize, glm::vec3{} };
		for (unsigned int i = 0; i < m_SSAOKernelSize; ++i)
		{
			samplePoints[i] = glm::vec3{
				RandomFloats(NumberGenerator)*2.0 - 1.0,
				RandomFloats(NumberGenerator)*2.0 - 1.0,
				RandomFloats(NumberGenerator)
			};
			samplePoints[i] = glm::normalize(samplePoints[i]);
			samplePoints[i] *= RandomFloats(NumberGenerator);

			float scale = float(i)/m_SSAOKernelSize;
			scale = lerp(0.1f, 1.0f, scale*scale);
			samplePoints[i] *= scale;
		}

		pDevice->AssignBuffer(m_SamplePointsDomeSSBO, samplePoints.data(), 0, sizeof(glm::vec3)*m_SSAOKernelSize);
	}

	void GloryRendererModule::GenerateNoiseTexture(GraphicsDevice* pDevice)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::GenerateNoiseTexture" };
		const size_t textureSize = 4;
		std::vector<glm::vec4> ssaoNoise;
		for (unsigned int i = 0; i < textureSize*textureSize; ++i)
		{
			glm::vec4 noise(
				RandomFloats(NumberGenerator)*2.0 - 1.0,
				RandomFloats(NumberGenerator)*2.0 - 1.0,
				0.0f, 0.0f);
			ssaoNoise.push_back(noise);
		}

		TextureCreateInfo textureInfo;
		textureInfo.m_Width = textureSize;
		textureInfo.m_Height = textureSize;
		textureInfo.m_PixelFormat = PixelFormat::PF_RGBA;
		textureInfo.m_InternalFormat = PixelFormat::PF_R16G16B16A16Sfloat;
		textureInfo.m_ImageType = ImageType::IT_2D;
		textureInfo.m_Type = DataType::DT_Float;
		textureInfo.m_ImageAspectFlags = ImageAspect::IA_Color;
		textureInfo.m_SamplerSettings.MipmapMode = Filter::F_None;
		m_SampleNoiseTexture = pDevice->CreateTexture(textureInfo, static_cast<const void*>(ssaoNoise.data()), sizeof(glm::vec4)*ssaoNoise.size());
	}

	void GloryRendererModule::ShadowMapsPass(CommandBufferHandle commandBuffer)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::ShadowMapsPass" };
		if (m_FrameData.ActiveLights.count() == 0) return;

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		GPUTextureAtlas& shadowAtlas = GetGPUTextureAtlas(m_ShadowAtlasses[m_CurrentFrameIndex]);
		pDevice->BeginRenderPass(commandBuffer, m_ShadowsPasses[m_CurrentFrameIndex]);
		for (size_t i = 0; i < m_FrameData.ActiveLights.count(); ++i)
		{
			auto& lightData = m_FrameData.ActiveLights[i];
			const auto& lightTransform = m_FrameData.LightSpaceTransforms[i];
			const auto& lightID = m_FrameData.ActiveLightIDs[i];
			if (!lightData.shadowsEnabled) continue;
			const glm::vec4 chunkRect = shadowAtlas.GetChunkPositionAndSize(lightID);
			RenderShadows(commandBuffer, i, chunkRect);
		}
		pDevice->EndRenderPass(commandBuffer);
	}

	void GloryRendererModule::RenderShadows(CommandBufferHandle commandBuffer, size_t lightIndex, const glm::vec4& viewport)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::RenderShadows" };
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		MaterialManager& materialManager = m_pEngine->GetMaterialManager();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		AssetManager& assets = m_pEngine->GetAssetManager();

		const bool usePushConstants = pDevice->IsSupported(APIFeatures::PushConstants);

		RenderConstants constants;
		constants.m_CameraIndex = static_cast<uint32_t>(lightIndex);
		constants.m_LightCount = m_FrameData.ActiveLights.count();
		if (!usePushConstants) pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));

		pDevice->BeginPipeline(commandBuffer, m_ShadowRenderPipeline);

		size_t batchIndex = 0;
		for (const PipelineBatch& pipelineRenderData : m_DynamicPipelineRenderDatas)
		{
			if (batchIndex >= m_DynamicBatchData.size()) break;
			const PipelineBatchData& batchData = m_DynamicBatchData.at(batchIndex);
			++batchIndex;

			if (viewport.z > 0.0f && viewport.w > 0.0f)
			{
				pDevice->SetViewport(commandBuffer, int(viewport.x), int(viewport.y), viewport.z, viewport.w);
				pDevice->SetScissor(commandBuffer, int(viewport.x), int(viewport.y), viewport.z, viewport.w);
			}

			pDevice->BindDescriptorSets(commandBuffer, batchData.m_Pipeline,
				{ m_GlobalShadowRenderSet, batchData.m_ObjectDataSet });

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
						pDevice->PushConstants(commandBuffer, batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
					else
						pDevice->AssignBuffer(m_RenderConstantsBuffer, &constants, sizeof(RenderConstants));
					pDevice->DrawMesh(commandBuffer, mesh);
				}
			}
		}

		pDevice->EndPipeline(commandBuffer);
	}

	void GloryRendererModule::OnSubmitCamera(CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::OnSubmitCamera" };
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		auto iter = std::find_if(m_ActiveCameras.begin(), m_ActiveCameras.end(),
			[camera](const CameraRef& other) { return other.GetUUID() == camera.GetUUID(); });
		if (iter == m_ActiveCameras.end()) return;
		const size_t cameraIndex = iter - m_ActiveCameras.begin();

		UniqueCameraData& cameraData = m_UniqueCameraDatas[camera.GetUUID()];
		cameraData.m_RenderPasses.resize(m_ImageCount, 0ull);
		cameraData.m_LateRenderPasses.resize(m_ImageCount, 0ull);
		cameraData.m_SSAORenderPasses.resize(m_ImageCount, 0ull);
		cameraData.m_LightIndexSSBOs.resize(m_ImageCount, 0ull);
		cameraData.m_LightGridSSBOs.resize(m_ImageCount, 0ull);
		cameraData.m_PickResultsSSBOs.resize(m_ImageCount, 0ull);
		cameraData.m_LightSets.resize(m_ImageCount, 0ull);
		cameraData.m_SSAOSamplersSets.resize(m_ImageCount, 0ull);
		cameraData.m_FinalColorSamplerSets.resize(m_ImageCount, 0ull);
		cameraData.m_PickingResultSets.resize(m_ImageCount, 0ull);
		cameraData.m_PickingSamplersSets.resize(m_ImageCount, 0ull);

		for (size_t i = 0; i < m_ImageCount; ++i)
		{
			RenderPassHandle& renderPass = cameraData.m_RenderPasses[i];
			RenderPassHandle& lateRenderPass = cameraData.m_LateRenderPasses[i];
			RenderPassHandle& ssaoRenderPass = cameraData.m_SSAORenderPasses[i];
			const auto& resolution = camera.GetResolution();
			if (!renderPass)
			{
				RenderPassInfo renderPassInfo;
				renderPassInfo.RenderTextureInfo.Width = resolution.x;
				renderPassInfo.RenderTextureInfo.Height = resolution.y;
				renderPassInfo.RenderTextureInfo.HasDepth = true;
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("object", PixelFormat::PF_RGBAI, PixelFormat::PF_R32G32B32A32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UInt, false));
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R16G16B16A16Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPassInfo.m_Position = RenderPassPosition::RP_Start;
				renderPassInfo.m_LoadOp = RenderPassLoadOp::OP_Clear;
				renderPass = pDevice->CreateRenderPass(std::move(renderPassInfo));
			}
			if (!lateRenderPass)
			{
				RenderPassInfo renderPassInfo;
				renderPassInfo.RenderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
				renderPassInfo.m_Position = RenderPassPosition::RP_Final;
				renderPassInfo.m_LoadOp = RenderPassLoadOp::OP_Load;
				lateRenderPass = pDevice->CreateRenderPass(std::move(renderPassInfo));
			}
			if (!ssaoRenderPass)
			{
				RenderPassInfo renderPassInfo;
				renderPassInfo.RenderTextureInfo.Width = resolution.x;
				renderPassInfo.RenderTextureInfo.Height = resolution.y;
				renderPassInfo.RenderTextureInfo.HasDepth = false;
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("AO", PixelFormat::PF_R, PixelFormat::PF_R32Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				ssaoRenderPass = pDevice->CreateRenderPass(std::move(renderPassInfo));
			}

			DescriptorSetHandle& lightSet = cameraData.m_LightSets[i];
			DescriptorSetHandle& ssaoSamplersSet = cameraData.m_SSAOSamplersSets[i];
			DescriptorSetHandle& colorSamplerSet = cameraData.m_FinalColorSamplerSets[i];
			DescriptorSetHandle& pickingResultsSet = cameraData.m_PickingResultSets[i];
			DescriptorSetHandle& pickingSamplersSet = cameraData.m_PickingSamplersSets[i];
			BufferHandle& lightIndexSSBO = cameraData.m_LightIndexSSBOs[i];
			BufferHandle& lightGridSSBO = cameraData.m_LightGridSSBOs[i];
			BufferHandle& pickResultsUBO = cameraData.m_PickResultsSSBOs[i];
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			TextureHandle objectID = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 1);
			TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 2);
			TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 3);

			if (!lightIndexSSBO)
				lightIndexSSBO = pDevice->CreateBuffer(sizeof(uint32_t)*(NUM_CLUSTERS*MAX_LIGHTS_PER_TILE + 1), BufferType::BT_Storage, BF_None);
			if (!lightGridSSBO)
				lightGridSSBO = pDevice->CreateBuffer(sizeof(LightGrid)*NUM_CLUSTERS, BufferType::BT_Storage, BF_None);
			if (!pickResultsUBO)
				pickResultsUBO = pDevice->CreateBuffer(sizeof(GPUPickResult)*MaxPicks + sizeof(uint32_t)*4, BufferType::BT_Storage, BF_Read);

			if (!lightSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_CameraLightSetLayout;
				setInfo.m_Buffers.resize(2);
				setInfo.m_Buffers[0].m_BufferHandle = lightIndexSSBO;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(uint32_t)*(NUM_CLUSTERS*MAX_LIGHTS_PER_TILE + 1);
				setInfo.m_Buffers[1].m_BufferHandle = lightGridSSBO;
				setInfo.m_Buffers[1].m_Offset = 0;
				setInfo.m_Buffers[1].m_Size = sizeof(LightGrid)*NUM_CLUSTERS;
				lightSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!ssaoSamplersSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_SSAOSamplersSetLayout;
				setInfo.m_Samplers.resize(2);
				setInfo.m_Samplers[0].m_TextureHandle = normals;
				setInfo.m_Samplers[1].m_TextureHandle = depth;
				ssaoSamplersSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!colorSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = color;
				colorSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!pickingResultsSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_PickingResultSetLayout;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = pickResultsUBO;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(GPUPickResult)*MaxPicks + sizeof(uint32_t)*4;
				pickingResultsSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!pickingSamplersSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_PickingSamplerSetLayout;
				setInfo.m_Samplers.resize(3);
				setInfo.m_Samplers[0].m_TextureHandle = objectID;
				setInfo.m_Samplers[1].m_TextureHandle = normals;
				setInfo.m_Samplers[2].m_TextureHandle = depth;
				pickingSamplersSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}
		}
	}

	void GloryRendererModule::OnUnsubmitCamera(CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::OnUnsubmitCamera" };
	}

	void GloryRendererModule::OnCameraUpdated(CameraRef camera)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::OnCameraUpdated" };
	}

	void GloryRendererModule::PrepareCameras()
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		/* Prepare cameras */
		if (m_CameraDatas->size() < m_ActiveCameras.size())
			m_CameraDatas.resize(m_ActiveCameras.size());
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];

			const PerCameraData cameraData{ camera.GetView(), camera.GetProjection(), camera.GetViewInverse(), camera.GetProjectionInverse(),
				camera.GetNear(), camera.GetFar(), { static_cast<glm::vec2>(camera.GetResolution()) }};

			if (std::memcmp(&m_CameraDatas.m_Data[i], &cameraData, sizeof(PerCameraData)) != 0)
			{
				std::memcpy(&m_CameraDatas.m_Data[i], &cameraData, sizeof(PerCameraData));
				m_CameraDatas.SetDirty(i);
			}
			m_UniqueCameraDatas.at(camera.GetUUID()).m_Picks.clear();
		}
		if (m_CameraDatas)
		{
			const size_t dirtySize = m_CameraDatas.DirtySize();
			pDevice->AssignBuffer(m_CameraDatasBuffer, m_CameraDatas.DirtyStart(),
				m_CameraDatas.m_DirtyRange.first*sizeof(PerCameraData), static_cast<uint32_t>(dirtySize*sizeof(PerCameraData)));
		}
	}

	void GloryRendererModule::GenerateShadowMapLODResolutions()
	{
		m_ShadowMapResolutions.reserve(m_MaxShadowLODs);
		for (size_t i = 0; i < m_MaxShadowLODs; ++i)
		{
			const uint32_t res = std::max(m_MinShadowResolution, m_MaxShadowResolution / m_ShadowLODDivisions[i]);
			m_ShadowMapResolutions.push_back({ res, res });
		}
	}

	void GloryRendererModule::ResizeShadowMapLODResolutions(uint32_t minSize, uint32_t maxSize)
	{
		m_MinShadowResolution = minSize;
		m_MaxShadowResolution = maxSize;
		m_ShadowMapResolutions.reserve(m_MaxShadowLODs);

		for (size_t i = 0; i < m_MaxShadowLODs; ++i)
		{
			const uint32_t res = std::max(m_MinShadowResolution, m_MaxShadowResolution / m_ShadowLODDivisions[i]);
			if (i >= m_ShadowMapResolutions.size())
			{
				m_ShadowMapResolutions.push_back({ res, res });
				continue;
			}
			m_ShadowMapResolutions[i] = { res, res };
		}
	}

	void GloryRendererModule::GenerateShadowLODDivisions(uint32_t maxLODs)
	{
		m_MaxShadowLODs = std::min(MAX_SHADOW_LODS, maxLODs);
		m_ShadowLODDivisions.clear();
		m_ShadowLODDivisions.reserve(m_MaxShadowLODs);
		for (size_t i = 0; i < m_MaxShadowLODs; ++i)
		{
			const uint32_t divider = uint32_t(pow(2, i));
			m_ShadowLODDivisions.push_back(divider);
		}
	}
}
