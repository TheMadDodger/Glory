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

	constexpr size_t AttachmentNameCount = 7;
	constexpr std::string_view AttachmentNames[AttachmentNameCount] = {
		"ObjectID",
		"Debug",
		"Color",
		"Normal",
		"AOBlurred",
		"Data",
		"AO",
	};

	constexpr size_t DebugOverlayNameCount = 1;
	constexpr std::string_view DebugOverlayNames[AttachmentNameCount] = {
		"Shadow Atlas",
	};

	GLORY_MODULE_VERSION_CPP(GloryRendererModule);

	struct BufferBindingIndices
	{
		static constexpr uint32_t RenderConstants = 0;
		static constexpr uint32_t CameraDatas = 1;
		static constexpr uint32_t WorldTransforms = 2;
		static constexpr uint32_t Materials = 3;
		static constexpr uint32_t HasTexture = 4;

		static constexpr uint32_t Clusters = 2;
		static constexpr uint32_t LightDatas = 3;
		static constexpr uint32_t LightSpaceTransforms = 4;
		static constexpr uint32_t LightIndices = 5;
		static constexpr uint32_t LightGrid = 6;
		static constexpr uint32_t LightDistances = 7;
		static constexpr uint32_t SampleDome = 2;
	};

	GloryRendererModule::GloryRendererModule()
	{
	}

	GloryRendererModule::~GloryRendererModule()
	{
	}

	void GloryRendererModule::OnCameraResize(CameraRef camera)
	{
		RenderPassHandle& renderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("RenderPass"));
		RenderPassHandle& ssaoRenderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("SSAORenderPass"));
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!renderPass || !pDevice) return;
		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
		RenderTextureHandle ssaoRenderTexture = pDevice->GetRenderPassRenderTexture(ssaoRenderPass);
		const glm::uvec2 resolution = camera.GetResolution();
		pDevice->ResizeRenderTexture(renderTexture, resolution.x, resolution.y);
		pDevice->ResizeRenderTexture(ssaoRenderTexture, resolution.x, resolution.y);

		/* Update descriptor sets */
		DescriptorSetHandle& ssaoSamplersSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("SSAOSamplersSet"));
		TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 3);
		TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 6);
		DescriptorSetUpdateInfo updateInfo;
		updateInfo.m_Samplers.resize(2);
		updateInfo.m_Samplers[0].m_TextureHandle = normals;
		updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
		updateInfo.m_Samplers[1].m_TextureHandle = depth;
		updateInfo.m_Samplers[1].m_DescriptorIndex = 1;
		pDevice->UpdateDescriptorSet(ssaoSamplersSet, updateInfo);
		/* When the camera rendertexture resizes we need to generate a new grid of clusters for that camera */
		OnCameraPerspectiveChanged(camera);
		camera.SetResolutionDirty(false);
	}

	void GloryRendererModule::OnCameraPerspectiveChanged(CameraRef camera)
	{
		/* When the camera changed perspective we need to generate a new grid of clusters for that camera */
		auto iter = std::find_if(m_ActiveCameras.begin(), m_ActiveCameras.end(),
			[camera](const CameraRef& other) { return other.GetUUID() == camera.GetUUID(); });
		if (iter == m_ActiveCameras.end()) return;
		const size_t cameraIndex = iter - m_ActiveCameras.begin();
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		DescriptorSetHandle clusterSet = camera.GetUserHandle("ClusterSet");
		if (!clusterSet) return; // Should not happen but just in case
		GenerateClusterSSBO(cameraIndex, pDevice, camera, clusterSet);
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
		m_pEngine->GetConsole().RegisterCVar({ std::string{ ScreenSpaceAOCVarName }, "Enables/disables screen space ambient occlusion.", float(m_GlobalSSAOSetting.m_Enabled), CVar::Flags::Save });

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ ScreenSpaceAOCVarName }, [this](const CVar* cvar) {
			m_GlobalSSAOSetting.m_Enabled = cvar->m_Value == 1.0f;
			m_GlobalSSAOSetting.m_Dirty = true;
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
					*pConstantsBuffer = pDevice->CreateBuffer(constantsSize, BufferType::BT_Uniform);
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
					*pConstantsBuffer = pDevice->CreateBuffer(constantsSize, BufferType::BT_Uniform);
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

		/* Global data buffers */
		m_CameraDatasBuffer = pDevice->CreateBuffer(sizeof(PerCameraData)*MAX_CAMERAS, BufferType::BT_Storage);
		m_LightCameraDatasBuffer = pDevice->CreateBuffer(sizeof(PerCameraData)*MAX_LIGHTS, BufferType::BT_Storage);
		m_LightsSSBO = pDevice->CreateBuffer(sizeof(LightData)*MAX_LIGHTS, BufferType::BT_Storage);
		m_LightSpaceTransformsSSBO = pDevice->CreateBuffer(sizeof(glm::mat4)*MAX_LIGHTS, BufferType::BT_Storage);

		GenerateDomeSamplePointsSSBO(pDevice, 64);
		GenerateNoiseTexture(pDevice);

		/* Global set */
		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Vertex }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalRenderSetLayout, m_GlobalRenderSet, &m_RenderConstantsBuffer, ShaderTypeFlag(STF_Vertex | STF_Fragment), 0, sizeof(RenderConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, usePushConstants, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Vertex }, { m_LightCameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_LIGHTS } },
			m_GlobalShadowRenderSetLayout, m_GlobalShadowRenderSet, &m_RenderConstantsBuffer, ShaderTypeFlag(STF_Vertex | STF_Fragment), 0, sizeof(RenderConstants));

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
		m_NoiseSamplerSet = CreateSamplerDescriptorSet(pDevice, 1, { m_SampleNoiseTexture }, m_NoiseSamplerSetLayout);

		/*ResetLightDistances = new uint32_t[MAX_LIGHTS];
		for (size_t i = 0; i < MAX_LIGHTS; ++i)
			ResetLightDistances[i] = NUM_DEPTH_SLICES;
		pDevice->AssignBuffer(m_LightDistancesSSBO, ResetLightDistances);*/

		RenderPassInfo shadowsPassInfo;
		shadowsPassInfo.RenderTextureInfo.EnableDepthStencilSampling = true;
		shadowsPassInfo.RenderTextureInfo.HasDepth = true;
		shadowsPassInfo.RenderTextureInfo.HasStencil = false;
		shadowsPassInfo.RenderTextureInfo.Width = 4096;
		shadowsPassInfo.RenderTextureInfo.Height = 4096;
		m_ShadowsPass = pDevice->CreateRenderPass(std::move(shadowsPassInfo));
		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_ShadowsPass);
		TextureHandle texture = pDevice->GetRenderTextureAttachment(renderTexture, 0);

		TextureCreateInfo info;
		info.m_Width = 4096;
		info.m_Height = 4096;
		m_pShadowAtlas = CreateGPUTextureAtlas(std::move(info), texture);
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
		const ModuleSettings& settings = Settings();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();

		//m_pEngine->GetDebug().SubmitLines(this, &m_pEngine->Time());
		//Render();

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		/* Make sure every camera has a render pass */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];

			RenderPassHandle& renderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("RenderPass"));
			RenderPassHandle& ssaoRenderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("SSAORenderPass"));
			//RenderPassHandle& deferredRenderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("DeferredRenderPass"));
			const auto& resolution = camera.GetResolution();
			if (!renderPass)
			{
				RenderPassInfo renderPassInfo;
				renderPassInfo.RenderTextureInfo.Width = resolution.x;
				renderPassInfo.RenderTextureInfo.Height = resolution.y;
				renderPassInfo.RenderTextureInfo.HasDepth = true;
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("object", PixelFormat::PF_RGBAI, PixelFormat::PF_R32G32B32A32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UInt, false));
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Debug", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R16G16B16A16Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("AOBlurred", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Data", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPass = pDevice->CreateRenderPass(std::move(renderPassInfo));
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
			if (!m_SSAOPipeline)
			{
				const UUID ssaoPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
				PipelineData* pPipeline = pipelines.GetPipelineData(ssaoPipeline);
				m_SSAOPipeline = pDevice->CreatePipeline(ssaoRenderPass, pPipeline, { m_GlobalClusterSetLayout, m_GlobalSampleDomeSetLayout, m_SSAOSamplersSetLayout, m_NoiseSamplerSetLayout },
					sizeof(glm::vec3), { AttributeType::Float3 });
			}

			/*if (!deferredRenderPass)
			{
				RenderPassInfo renderPassInfo;
				renderPassInfo.RenderTextureInfo.Width = resolution.x;
				renderPassInfo.RenderTextureInfo.Height = resolution.y;
				renderPassInfo.RenderTextureInfo.HasDepth = false;
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				deferredRenderPass = pDevice->CreateRenderPass(renderPassInfo);
			}*/
		}

		m_PickResults.clear();

		ProfileSample s{ &m_pEngine->Profiler(), "RendererModule::Render" };

		PrepareDataPass();

		m_CommandBuffer = pDevice->Begin();

		/* Shadows */
		ShadowMapsPass();

		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];

			RenderPassHandle& renderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("RenderPass"));
			RenderPassHandle& ssaoRenderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("SSAORenderPass"));
			DescriptorSetHandle& ssaoSamplersSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("SSAOSamplersSet"));
			//RenderPassHandle& deferredRenderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("DeferredRenderPass"));
			/* Light cluster culling */
			ClusterPass(static_cast<uint32_t>(i));
			
			/* Draw objects */
			pDevice->SetRenderPassClear(renderPass, camera.GetClearColor());
			pDevice->BeginRenderPass(m_CommandBuffer, renderPass);
			DynamicObjectsPass(static_cast<uint32_t>(i));
			pDevice->EndRenderPass(m_CommandBuffer);

			SSAOConstants constants;
			constants.CameraIndex = i;
			constants.KernelSize = m_GlobalSSAOSetting.m_KernelSize;
			constants.SampleRadius = m_GlobalSSAOSetting.m_SampleRadius;
			constants.SampleBias = m_GlobalSSAOSetting.m_SampleBias;

			const glm::uvec2& resolution = camera.GetResolution();

			pDevice->BeginRenderPass(m_CommandBuffer, ssaoRenderPass);
			pDevice->BeginPipeline(m_CommandBuffer, m_SSAOPipeline);
			pDevice->SetViewport(m_CommandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
			pDevice->SetScissor(m_CommandBuffer, 0, 0, resolution.x, resolution.y);
			pDevice->BindDescriptorSets(m_CommandBuffer, m_SSAOPipeline, { m_SSAOCameraSet, m_GlobalSampleDomeSet, ssaoSamplersSet, m_NoiseSamplerSet });
			if (!m_SSAOConstantsBuffer)
				pDevice->PushConstants(m_CommandBuffer, m_SSAOPipeline, 0, sizeof(SSAOConstants), &constants, STF_Fragment);
			else
				pDevice->AssignBuffer(m_SSAOConstantsBuffer, &constants, sizeof(SSAOConstants));
			pDevice->DrawQuad(m_CommandBuffer);
			pDevice->EndPipeline(m_CommandBuffer);
			pDevice->EndRenderPass(m_CommandBuffer);

			/* Deferred composite */
			//pDevice->BeginRenderPass(m_CommandBuffer, deferredRenderPass);

			//pDevice->EndRenderPass(m_CommandBuffer);
		}

		pDevice->End(m_CommandBuffer);
		pDevice->Commit(m_CommandBuffer);
		pDevice->Wait(m_CommandBuffer);
		pDevice->Release(m_CommandBuffer);

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

	size_t GloryRendererModule::DefaultAttachmenmtIndex() const
	{
		return 2;
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
		RenderPassHandle ssaoRenderPass = camera.GetUserHandle("SSAORenderPass");
		if (!renderPass || !ssaoRenderPass) return NULL;
		if (index >= 6)
		{
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(ssaoRenderPass);
			return pDevice->GetRenderTextureAttachment(renderTexture, index-6);
		}

		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
		return pDevice->GetRenderTextureAttachment(renderTexture, index);
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
			return m_pShadowAtlas->GetTexture();
		}
		return NULL;
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

	void GloryRendererModule::RenderBatches(const std::vector<PipelineBatch>& batches, const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex, DescriptorSetHandle globalRenderSet, const glm::vec4& viewport)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		MaterialManager& materialManager = m_pEngine->GetMaterialManager();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		AssetManager& assets = m_pEngine->GetAssetManager();

		const bool usePushConstants = pDevice->IsSupported(APIFeatures::PushConstants);

		RenderConstants constants;
		constants.m_CameraIndex = static_cast<uint32_t>(cameraIndex);
		CameraRef camera = m_ActiveCameras[cameraIndex];
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
			pDevice->BeginPipeline(m_CommandBuffer, batchData.m_Pipeline);
			if (viewport.z > 0.0f && viewport.w > 0.0f)
			{
				pDevice->SetViewport(m_CommandBuffer, int(viewport.x), int(viewport.y), viewport.z, viewport.w);
				pDevice->SetScissor(m_CommandBuffer, int(viewport.x), int(viewport.y), viewport.z, viewport.w);
			}

			pDevice->BindDescriptorSets(m_CommandBuffer, batchData.m_Pipeline, { globalRenderSet, batchData.m_Set });

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
						pDevice->PushConstants(m_CommandBuffer, batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
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
		if (!m_ClusterGeneratorPipeline)
		{
			PipelineData* pPipeline = pipelines.GetPipelineData(clusterGeneratorPipeline);
			m_ClusterGeneratorPipeline = pDevice->CreateComputePipeline(pPipeline, { m_GlobalClusterSetLayout, m_CameraClusterSetLayout });
		}
		if (!m_ClusterCullLightPipeline)
		{
			PipelineData* pPipeline = pipelines.GetPipelineData(clusterCullLightPipeline);
			m_ClusterCullLightPipeline = pDevice->CreateComputePipeline(pPipeline,
				{ m_GlobalClusterSetLayout, m_CameraClusterSetLayout, m_GlobalLightSetLayout, m_CameraLightSetLayout });
		}

		MaterialManager& materials = m_pEngine->GetMaterialManager();

		/* Prepare cameras */
		if (m_CameraDatas->size() < m_ActiveCameras.size())
			m_CameraDatas.resize(m_ActiveCameras.size());
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];

			const PerCameraData cameraData{ camera.GetView(), camera.GetProjection(),
				camera.GetNear(), camera.GetFar(), { static_cast<glm::vec2>(camera.GetResolution()) }};

			if (std::memcmp(&m_CameraDatas.m_Data[i], &cameraData, sizeof(PerCameraData)) != 0)
			{
				std::memcpy(&m_CameraDatas.m_Data[i], &cameraData, sizeof(PerCameraData));
				m_CameraDatas.m_Dirty = true;
			}

			BufferHandle& clusterSSBOHandle = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("ClusterSSBO"));
			if (!clusterSSBOHandle)
			{
				DescriptorSetHandle& clusterSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("ClusterSet"));
				DescriptorSetHandle& lightSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("LightSet"));
				DescriptorSetHandle& ssaoSamplersSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("SSAOSamplersSet"));
				BufferHandle& lightIndexSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightIndexSSBO"));
				BufferHandle& lightGridSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightGridSSBO"));
				BufferHandle& lightDistancesSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightDistancesSSBO"));
				RenderPassHandle& renderPass = reinterpret_cast<RenderPassHandle&>(camera.GetUserHandle("RenderPass"));
				RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
				TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 3);
				TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 6);

				clusterSSBOHandle = pDevice->CreateBuffer(sizeof(VolumeTileAABB)*NUM_CLUSTERS, BufferType::BT_Storage);
				lightIndexSSBO = pDevice->CreateBuffer(sizeof(uint32_t)*(NUM_CLUSTERS*MAX_LIGHTS_PER_TILE + 1), BufferType::BT_Storage);
				lightGridSSBO = pDevice->CreateBuffer(sizeof(LightGrid)*NUM_CLUSTERS, BufferType::BT_Storage);
				lightDistancesSSBO = pDevice->CreateBuffer(sizeof(uint32_t)*MAX_LIGHTS, BufferType::BT_Storage);

				DescriptorSetInfo setInfo;
				setInfo.m_Layout = m_CameraClusterSetLayout;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = clusterSSBOHandle;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(VolumeTileAABB)*NUM_CLUSTERS;
				clusterSet = pDevice->CreateDescriptorSet(std::move(setInfo));

				setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_CameraLightSetLayout;
				setInfo.m_Buffers.resize(3);
				setInfo.m_Buffers[0].m_BufferHandle = lightIndexSSBO;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(uint32_t)*(NUM_CLUSTERS*MAX_LIGHTS_PER_TILE + 1);
				setInfo.m_Buffers[1].m_BufferHandle = lightGridSSBO;
				setInfo.m_Buffers[1].m_Offset = 0;
				setInfo.m_Buffers[1].m_Size = sizeof(LightGrid)*NUM_CLUSTERS;
				setInfo.m_Buffers[2].m_BufferHandle = lightDistancesSSBO;
				setInfo.m_Buffers[2].m_Offset = 0;
				setInfo.m_Buffers[2].m_Size = sizeof(uint32_t)*MAX_LIGHTS;
				lightSet = pDevice->CreateDescriptorSet(std::move(setInfo));
				
				setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_SSAOSamplersSetLayout;
				setInfo.m_Samplers.resize(2);
				setInfo.m_Samplers[0].m_TextureHandle = normals;
				setInfo.m_Samplers[1].m_TextureHandle = depth;
				ssaoSamplersSet = pDevice->CreateDescriptorSet(std::move(setInfo));

				GenerateClusterSSBO(i, pDevice, camera, clusterSet);

				auto iter = std::find(m_DirtyCameraPerspectives.begin(), m_DirtyCameraPerspectives.end(), camera);
				if (iter != m_DirtyCameraPerspectives.end())
					m_DirtyCameraPerspectives.erase(iter);
			}
		}
		if (m_CameraDatas)
			pDevice->AssignBuffer(m_CameraDatasBuffer, m_CameraDatas->data(),
				static_cast<uint32_t>(m_CameraDatas->size()*sizeof(PerCameraData)));

		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];
			DescriptorSetHandle& clusterSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("ClusterSet"));
			if (std::find(m_DirtyCameraPerspectives.begin(), m_DirtyCameraPerspectives.end(), camera) != m_DirtyCameraPerspectives.end())
				GenerateClusterSSBO(i, pDevice, camera, clusterSet);
		}
		m_DirtyCameraPerspectives.clear();

		/* Update light data */
		pDevice->AssignBuffer(m_LightsSSBO, m_FrameData.ActiveLights.data(), 0, MAX_LIGHTS*sizeof(LightData));
		pDevice->AssignBuffer(m_LightSpaceTransformsSSBO, m_FrameData.LightSpaceTransforms.data(), 0, MAX_LIGHTS*sizeof(glm::mat4));

		if (m_LightCameraDatas->size() < m_FrameData.LightSpaceTransforms.count()) m_LightCameraDatas.resize(m_FrameData.LightSpaceTransforms.count());
		for (size_t i = 0; i < m_FrameData.LightSpaceTransforms.count(); ++i)
		{
			m_LightCameraDatas.m_Data[i].m_View = glm::identity<glm::mat4>();
			if (m_LightCameraDatas.m_Data[i].m_Projection != m_FrameData.LightSpaceTransforms[i])
			{
				m_LightCameraDatas.m_Data[i].m_Projection = m_FrameData.LightSpaceTransforms[i];
				m_LightCameraDatas.m_Dirty = true;
			}
		}
		if (m_LightCameraDatas)
			pDevice->AssignBuffer(m_LightCameraDatasBuffer, m_LightCameraDatas->data(), m_LightCameraDatas->size()*sizeof(PerCameraData));

		PrepareBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData);
	}

	void GloryRendererModule::PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas)
	{
		if (m_ActiveCameras.empty()) return;
		CameraRef defaultCamera = m_ActiveCameras[0];
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
				setLayoutInfo.m_Buffers[0].m_BindingIndex = uint32_t(BufferBindingIndices::WorldTransforms);
				setLayoutInfo.m_Buffers[0].m_Type = BT_Storage;
				setLayoutInfo.m_Buffers[0].m_ShaderStages = STF_Vertex;
				setInfo.m_Buffers[0].m_BufferHandle = batchData.m_WorldsBuffer;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = batchData.m_Worlds->size()*sizeof(glm::mat4);

				setLayoutInfo.m_Buffers[1].m_BindingIndex = uint32_t(BufferBindingIndices::Materials);
				setLayoutInfo.m_Buffers[1].m_Type = BT_Storage;
				setLayoutInfo.m_Buffers[1].m_ShaderStages = STF_Fragment;
				setInfo.m_Buffers[1].m_BufferHandle = batchData.m_MaterialsBuffer;
				setInfo.m_Buffers[1].m_Offset = 0;
				setInfo.m_Buffers[1].m_Size = batchData.m_MaterialDatas->size();
				if (batchData.m_TextureBitsBuffer)
				{
					setLayoutInfo.m_Buffers[2].m_BindingIndex = uint32_t(BufferBindingIndices::HasTexture);
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
				descriptorSetLayouts[0] = m_GlobalRenderSetLayout;
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

	void GloryRendererModule::GenerateClusterSSBO(uint32_t cameraIndex, GraphicsDevice* pDevice, CameraRef camera, DescriptorSetHandle clusterSet)
	{
		const glm::uvec2 resolution = camera.GetResolution();
		const glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		const float zNear = camera.GetNear();
		const float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x/(float)gridSize.x), (uint32_t)std::ceilf(resolution.y/(float)gridSize.y));
		ClusterConstants constants;
		constants.CameraIndex = cameraIndex;
		constants.LightCount = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		constants.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		constants.Scale = (float)gridSize.z/std::log2f(zFar/zNear);
		constants.Bias = -((float)gridSize.z*std::log2f(zNear)/std::log2f(zFar/zNear));

		CommandBufferHandle commandBuffer = pDevice->Begin();
		pDevice->BeginPipeline(commandBuffer, m_ClusterGeneratorPipeline);
		pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
		pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
		pDevice->BindDescriptorSets(commandBuffer, m_ClusterGeneratorPipeline, { m_GlobalClusterSet, clusterSet });
		if (!m_ClusterConstantsBuffer)
			pDevice->PushConstants(commandBuffer, m_ClusterGeneratorPipeline, 0, sizeof(ClusterConstants), &constants, STF_Compute);
		else
			pDevice->AssignBuffer(m_ClusterConstantsBuffer, &constants, sizeof(ClusterConstants));
		pDevice->Dispatch(commandBuffer, gridSize.x, gridSize.y, gridSize.z);
		pDevice->EndPipeline(commandBuffer);
		pDevice->End(commandBuffer);
		pDevice->Commit(commandBuffer);
	}

	void GloryRendererModule::ClusterPass(uint32_t cameraIndex)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();

		CameraRef camera = m_ActiveCameras[cameraIndex];
		const DescriptorSetHandle& clusterSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("ClusterSet"));
		const DescriptorSetHandle& lightSet = reinterpret_cast<DescriptorSetHandle&>(camera.GetUserHandle("LightSet"));
		const BufferHandle& lightIndexSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightIndexSSBO"));
		const BufferHandle& lightGridSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightGridSSBO"));
		const BufferHandle& lightDistancesSSBO = reinterpret_cast<BufferHandle&>(camera.GetUserHandle("LightDistancesSSBO"));

		const glm::uvec2 resolution = camera.GetResolution();
		const glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x/(float)gridSize.x), (uint32_t)std::ceilf(resolution.y/(float)gridSize.y));
		ClusterConstants constants;
		constants.CameraIndex = cameraIndex;
		constants.LightCount = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		constants.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		constants.Scale = (float)gridSize.z/std::log2f(zFar/zNear);
		constants.Bias = -((float)gridSize.z*std::log2f(zNear)/std::log2f(zFar/zNear));

		pDevice->BeginPipeline(m_CommandBuffer, m_ClusterCullLightPipeline);
		pDevice->SetViewport(m_CommandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
		pDevice->SetScissor(m_CommandBuffer, 0, 0, resolution.x, resolution.y);
		pDevice->BindDescriptorSets(m_CommandBuffer, m_ClusterCullLightPipeline, { m_GlobalClusterSet, clusterSet, m_GlobalLightSet, lightSet });
		if (!m_ClusterConstantsBuffer)
			pDevice->PushConstants(m_CommandBuffer, m_ClusterCullLightPipeline, 0, sizeof(ClusterConstants), &constants, STF_Compute);
		else
			pDevice->AssignBuffer(m_ClusterConstantsBuffer, &constants, sizeof(ClusterConstants));
		pDevice->Dispatch(m_CommandBuffer, 1, 1, 6);
		pDevice->EndPipeline(m_CommandBuffer);
		pDevice->PipelineBarrier(m_CommandBuffer, { lightIndexSSBO, lightGridSSBO, lightDistancesSSBO }, {},
			PipelineStageFlagBits::PST_ComputeShader, PipelineStageFlagBits::PST_FragmentShader);
	}

	void GloryRendererModule::DynamicObjectsPass(uint32_t cameraIndex)
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		CameraRef camera = m_ActiveCameras[cameraIndex];
		//pGraphics->EnableDepthWrite(true);
		RenderBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData, cameraIndex, m_GlobalRenderSet, { 0.0f, 0.0f, camera.GetResolution() });
		//pGraphics->EnableDepthWrite(true);
	}

	std::uniform_real_distribution<float> RandomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine NumberGenerator;

	void GloryRendererModule::GenerateDomeSamplePointsSSBO(GraphicsDevice* pDevice, uint32_t size)
	{
		if (m_SSAOKernelSize == size) return;
		m_SSAOKernelSize = size;

		if (!m_SamplePointsDomeSSBO)
			m_SamplePointsDomeSSBO = pDevice->CreateBuffer(sizeof(glm::vec3)*MAX_KERNEL_SIZE, BufferType::BT_Uniform);

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

	void GloryRendererModule::ShadowMapsPass()
	{
		if (m_FrameData.ActiveLights.count() == 0) return;

		//uint32_t lightDepthSlices[MAX_LIGHTS];
		//m_LightDistancesSSBO->Read(&lightDepthSlices, 0, m_FrameData.ActiveLights.count() * sizeof(uint32_t));
		//GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		//const uint32_t sliceSteps = NUM_DEPTH_SLICES / m_MaxShadowLODs;

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		m_pShadowAtlas->ReleaseAllChunks();
		pDevice->BeginRenderPass(m_CommandBuffer, m_ShadowsPass);

		//pGraphics->SetCullFace(CullFace::Front);
		//pGraphics->SetColorMask(false, false, false, false);
		//pGraphics->EnableDepthWrite(true);
		//pGraphics->EnableDepthTest(true);

		for (size_t i = 0; i < m_FrameData.ActiveLights.count(); ++i)
		{
			auto& lightData = m_FrameData.ActiveLights[i];
			const auto& lightTransform = m_FrameData.LightSpaceTransforms[i];
			const auto& lightID = m_FrameData.ActiveLightIDs[i];

			if (!lightData.shadowsEnabled) continue;

			//const uint32_t depthSlice = lightDepthSlices[i];
			///* No need to render that which can't be seen! */
			//if (depthSlice == NUM_DEPTH_SLICES)
			//{
			//	lightData.shadowsEnabled = 0;
			//	continue;
			//}

			//const uint32_t shadowLOD = std::min(depthSlice / sliceSteps, uint32_t(m_MaxShadowLODs - 1));
			//const glm::uvec2 shadowMapResolution = m_ShadowMapResolutions[shadowLOD];
			const glm::uvec2 shadowMapResolution = glm::uvec2(512, 512);

			const UUID chunkID = m_pShadowAtlas->ReserveChunk(shadowMapResolution.x, shadowMapResolution.y, lightID);
			if (!chunkID)
			{
				lightData.shadowsEnabled = 0;
				m_pEngine->GetDebug().LogError("Failed to reserve chunk in shadow atlas, there is not enough space left.");
				continue;
			}

			const glm::vec4 chunkRect = m_pShadowAtlas->GetChunkPositionAndSize(chunkID);
			RenderShadows(i, chunkRect);
			lightData.shadowCoords = m_pShadowAtlas->GetChunkCoords(lightID);
		}

		//pGraphics->SetColorMask(true, true, true, true);
		//pGraphics->SetCullFace(CullFace::None);

		pDevice->EndRenderPass(m_CommandBuffer);
	}

	void GloryRendererModule::RenderShadows(size_t lightIndex, const glm::vec4& viewport)
	{
		//RenderBatches(m_StaticPipelineRenderDatas, m_StaticBatchData, lightIndex);
		RenderBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData, lightIndex, m_GlobalShadowRenderSet, viewport);
		//RenderBatches(m_DynamicLatePipelineRenderDatas, m_DynamicLateBatchData, lightIndex);
	}

	void GloryRendererModule::OnSubmitCamera(CameraRef camera)
	{

	}

	void GloryRendererModule::OnUnsubmitCamera(CameraRef camera)
	{
	}

	void GloryRendererModule::OnCameraUpdated(CameraRef camera)
	{
	}
}
