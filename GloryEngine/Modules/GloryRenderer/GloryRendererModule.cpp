#include "GloryRendererModule.h"

#include <Engine.h>
#include <Console.h>
#include <GraphicsDevice.h>
#include <DescriptorHelpers.h>
#include <GPUTextureAtlas.h>

#include <PipelineManager.h>
#include <MaterialManager.h>
#include <AssetManager.h>
#include <SceneManager.h>
#include <GScene.h>

#include <PipelineData.h>
#include <MeshData.h>
#include <TextureData.h>
#include <CubemapData.h>

#include <EngineProfiler.h>
#include <RenderHelpers.h>
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

	static constexpr std::string_view CameraOutputAttachment = "r_cameraOutputAttachment";
	static constexpr std::string_view VisualizeShadowAtlas = "r_visualizeShadowAtlas";
	static constexpr std::string_view VisualizeLightComplexity = "r_visualizeLightComplexity";

	static uint32_t* ResetLightDistances;

	constexpr size_t AttachmentNameCount = 6;
	constexpr std::string_view AttachmentNames[AttachmentNameCount] = {
		"ObjectID",
		"Color",
		"Normal",
		"AO",
		"Depth",
		"Final",
	};

	constexpr size_t DebugOverlayNameCount = 2;
	constexpr std::string_view DebugOverlayNames[AttachmentNameCount] = {
		"Shadow Atlas",
		"Light Complexity"
	};

	enum DebugOverlayBitIndices : uint8_t
	{
		ShadowAtlas = 0,
		LightComplexity = 1
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

	struct LightComplexityConstants
	{
		glm::uvec4 GridSize;
		glm::uvec2 Resolution;
		float zNear;
		float zFar;
	};

	struct OrderedObject
	{
		float minDistance;
		UUID meshID;
		uint32_t meshObjectIndex;
		uint32_t objectIndex;

		bool operator>(const OrderedObject& other) const
		{
			return minDistance < other.minDistance;
		}

		bool operator<(const OrderedObject& other) const
		{
			return minDistance > other.minDistance;
		}

		bool operator==(const OrderedObject& other) const
		{
			return minDistance == other.minDistance;
		}

		bool operator!=(const OrderedObject& other) const
		{
			return minDistance != other.minDistance;
		}

		bool operator>=(const OrderedObject& other) const
		{
			return minDistance <= other.minDistance;
		}

		bool operator<=(const OrderedObject& other) const
		{
			return minDistance >= other.minDistance;
		}
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
			const PostProcessPass& postProcessPass = uniqueCameraData.m_PostProcessPasses[i];
			if (!renderPass) continue;
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			RenderTextureHandle ssaoRenderTexture = pDevice->GetRenderPassRenderTexture(ssaoRenderPass);
			RenderTextureHandle finalColorBackBuffer = pDevice->GetRenderPassRenderTexture(postProcessPass.m_BackBufferPass);
			RenderTextureHandle finalColorFrontBuffer = pDevice->GetRenderPassRenderTexture(postProcessPass.m_FrontBufferPass);
			const glm::uvec2 resolution = camera.GetResolution();
			pDevice->ResizeRenderTexture(renderTexture, resolution.x, resolution.y);
			pDevice->ResizeRenderTexture(ssaoRenderTexture, resolution.x, resolution.y);
			pDevice->ResizeRenderTexture(finalColorBackBuffer, resolution.x, resolution.y);
			pDevice->ResizeRenderTexture(finalColorFrontBuffer, resolution.x, resolution.y);

			/* Update descriptor sets */
			const DescriptorSetHandle& ssaoSamplersSet = uniqueCameraData.m_SSAOSamplersSets[i];
			const DescriptorSetHandle& ssaoPostSamplersSet = uniqueCameraData.m_SSAOPostSamplersSets[i];
			const DescriptorSetHandle& colorSamplerSet = uniqueCameraData.m_ColorSamplerSets[i];
			const DescriptorSetHandle& pickingSamplersSet = uniqueCameraData.m_PickingSamplersSets[i];
			const DescriptorSetHandle& objectIDSamplerSet = uniqueCameraData.m_ObjectIDSamplerSets[i];
			const DescriptorSetHandle& normalSamplerSet = uniqueCameraData.m_NormalSamplerSets[i];
			const DescriptorSetHandle& aoSamplerSet = uniqueCameraData.m_AOSamplerSets[i];
			const DescriptorSetHandle& depthSamplerSet = uniqueCameraData.m_DepthSamplerSets[i];
			TextureHandle objectID = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 1);
			TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 2);
			TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 3);
			TextureHandle ao = pDevice->GetRenderTextureAttachment(ssaoRenderTexture, 0);
			TextureHandle ppBack = pDevice->GetRenderTextureAttachment(finalColorBackBuffer, 0);
			TextureHandle ppFront = pDevice->GetRenderTextureAttachment(finalColorFrontBuffer, 0);

			DescriptorSetUpdateInfo updateInfo;
			updateInfo.m_Samplers.resize(2);
			updateInfo.m_Samplers[0].m_TextureHandle = normals;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			updateInfo.m_Samplers[1].m_TextureHandle = depth;
			updateInfo.m_Samplers[1].m_DescriptorIndex = 1;
			pDevice->UpdateDescriptorSet(ssaoSamplersSet, updateInfo);

			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = ao;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(ssaoPostSamplersSet, updateInfo);

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

			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = ppBack;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(postProcessPass.m_BackDescriptor, updateInfo);

			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = ppFront;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(postProcessPass.m_FrontDescriptor, updateInfo);
			
			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = normals;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(normalSamplerSet, updateInfo);
			
			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = ao;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(aoSamplerSet, updateInfo);
			
			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = objectID;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(objectIDSamplerSet, updateInfo);
			
			updateInfo = DescriptorSetUpdateInfo();
			updateInfo.m_Samplers.resize(1);
			updateInfo.m_Samplers[0].m_TextureHandle = depth;
			updateInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(depthSamplerSet, updateInfo);
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
		newReferences.push_back(settings.Value<uint64_t>("SSAO Postpass"));
		newReferences.push_back(settings.Value<uint64_t>("SSAO Visualizer"));
		newReferences.push_back(settings.Value<uint64_t>("ObjectID Visualizer"));
		newReferences.push_back(settings.Value<uint64_t>("Depth Visualizer"));
		newReferences.push_back(settings.Value<uint64_t>("Light Complexity Visualizer"));

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

	uint32_t GloryRendererModule::GetNumFramesInFlight() const
	{
		return m_ImageCount;
	}

	uint32_t GloryRendererModule::GetCurrentFrameInFlight() const
	{
		return m_CurrentFrameIndex;
	}

	void GloryRendererModule::Initialize()
	{
		RendererModule::Initialize();
		m_pEngine->GetConsole().RegisterCVar({ std::string{ ScreenSpaceAOCVarName }, "Enables/disables screen space ambient occlusion.",
			float(m_GlobalSSAOSetting.m_Enabled), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MinShadowResolutionVarName }, "Sets the minimum resolution for shadow maps.",
			float(m_MinShadowResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MaxShadowResolutionVarName }, "Sets the maximum resolution for shadow maps.",
			float(m_MaxShadowResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ ShadowAtlasResolution }, "Sets the resolution for the shadow atlas.",
			float(m_ShadowAtlasResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MaxShadowLODs }, "Sets the number of shadow map LODs.",
			float(m_MaxShadowLODs), CVar::Flags::Save });

		m_pEngine->GetConsole().RegisterCVar({ std::string{ CameraOutputAttachment }, "Sets which attachment on the camera should be outputed.",
			float(DefaultAttachmenmtIndex()), CVar::Flags::None });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ VisualizeShadowAtlas }, "Enables/disables shadow atlas debug overlay.",
			float(DefaultAttachmenmtIndex()), CVar::Flags::None });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ VisualizeLightComplexity }, "Enables/disables light complexity debug overlay.",
			float(DefaultAttachmenmtIndex()), CVar::Flags::None });

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

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ CameraOutputAttachment }, [this](const CVar* cvar) {
			for (size_t i = 0; i < m_OutputCameras.size(); ++i)
				VisualizeAttachment(m_OutputCameras[i], size_t(cvar->m_Value));
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ VisualizeShadowAtlas }, [this](const CVar* cvar) {
			SetDebugOverlayEnabled(NULL, DebugOverlayBitIndices::ShadowAtlas, cvar->m_Value != 0.0f);
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ VisualizeLightComplexity }, [this](const CVar* cvar) {
			SetDebugOverlayEnabled(NULL, DebugOverlayBitIndices::LightComplexity, cvar->m_Value != 0.0f);
		});
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
		const UUID ssaoPostpassPipeline = settings.Value<uint64_t>("SSAO Postpass");
		const UUID objectIDVisualizerPipeline = settings.Value<uint64_t>("ObjectID Visualizer");
		const UUID depthVisualizerPipeline = settings.Value<uint64_t>("Depth Visualizer");
		const UUID lightComplexityVisualizerPipeline = settings.Value<uint64_t>("Light Complexity Visualizer");

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice)
		{
			m_pEngine->GetDebug().LogError("Renderer: No graphics device active");
			return;
		}

		/* Global data buffers */
		m_CameraDatasBuffer = pDevice->CreateBuffer(sizeof(PerCameraData)*MAX_CAMERAS, BufferType::BT_Storage, BF_Write);
		m_LightCameraDatasBuffer = pDevice->CreateBuffer(sizeof(PerCameraData)*MAX_LIGHTS, BufferType::BT_Storage, BF_Write);
		m_LightsSSBO = pDevice->CreateBuffer(sizeof(LightData)*MAX_LIGHTS, BufferType::BT_Storage, BF_Write);
		m_LightSpaceTransformsSSBO = pDevice->CreateBuffer(sizeof(glm::mat4)*MAX_LIGHTS, BufferType::BT_Storage, BF_Write);

		GenerateDomeSamplePointsSSBO(pDevice, 64);
		GenerateNoiseTexture(pDevice);

		/* Global set */
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Vertex | STF_Fragment) }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalRenderSetLayout, m_GlobalRenderSet, ShaderTypeFlag(STF_Vertex | STF_Fragment), 0, sizeof(RenderConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Vertex | STF_Fragment) }, { m_LightCameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_LIGHTS } },
			m_GlobalShadowRenderSetLayout, m_GlobalShadowRenderSet, ShaderTypeFlag(STF_Vertex | STF_Fragment), 0, sizeof(RenderConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Compute }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalPickingSetLayout, m_GlobalPickingSet, STF_Compute, 0, sizeof(PickingConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Vertex }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalLineRenderSetLayout, m_GlobalLineRenderSet, STF_Vertex, 0, sizeof(uint32_t));
		
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Vertex }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalSkyboxRenderSetLayout, m_GlobalSkyboxRenderSet, STF_Vertex, 0, sizeof(uint32_t));

		assert(m_GlobalRenderSetLayout == m_GlobalShadowRenderSetLayout);

		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalClusterSetLayout, m_GlobalClusterSet, ShaderTypeFlag(STF_Compute | STF_Fragment), 0, sizeof(ClusterConstants));

		m_CameraClusterSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::Clusters },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) });

		CreateBufferDescriptorLayoutAndSet(pDevice, 2, { BufferBindingIndices::LightDatas, BufferBindingIndices::LightSpaceTransforms },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) }, { m_LightsSSBO, m_LightSpaceTransformsSSBO }, { { 0, sizeof(LightData)*MAX_LIGHTS }, { 0, sizeof(glm::mat4)*MAX_LIGHTS } },
			m_GlobalLightSetLayout, m_GlobalLightSet);

		m_CameraLightSetLayout = CreateBufferDescriptorLayout(pDevice, 3, { BufferBindingIndices::LightIndices, BufferBindingIndices::LightGrid, BufferBindingIndices::LightDistances },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) });

		m_SSAOCameraSet = CreateBufferDescriptorSet(pDevice, 1, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData)*MAX_CAMERAS } },
			m_GlobalClusterSetLayout, 0, sizeof(SSAOConstants));

		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::SampleDome },
			{ BufferType::BT_Uniform }, { STF_Fragment }, { m_SamplePointsDomeSSBO },
			{ {0, sizeof(glm::vec3)*m_SSAOKernelSize} }, m_GlobalSampleDomeSetLayout, m_GlobalSampleDomeSet);

		m_SSAOSamplersSetLayout = CreateSamplerDescriptorLayout(pDevice, 2, { 0, 1 }, { STF_Fragment }, { "Normal", "Depth" });
		m_NoiseSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 2 }, { STF_Fragment }, { "Noise" });
		m_CameraSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 6, { 0, 1, 2, 3, 4, 5 }, { STF_Fragment }, { "Color", "Normal", "Depth" });
		m_NoiseSamplerSet = CreateSamplerDescriptorSet(pDevice, 1, { m_SampleNoiseTexture }, m_NoiseSamplerSetLayout);
		m_DisplayCopySamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "Color" });
		m_SSAOPostSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 1 }, { STF_Fragment }, { "AO" });
		m_ShadowAtlasSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "ShadowAtlas" });
		m_ObjectDataSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::WorldTransforms }, { BT_Storage }, { STF_Vertex });
		m_LightDistancesSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::LightDistances }, { BT_Storage }, { STF_Compute });

		m_PickingResultSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::PickingResults }, { BT_Storage }, { STF_Compute });
		m_PickingSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 3, { 0, 1, 2 }, { STF_Compute }, { "ObjectID", "Normal", "Depth" });
		m_GlobalSkyboxSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "Skybox" });
		m_LightGridSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { 1 }, { BT_Storage }, { STF_Fragment });

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

		/* Dummy render passes */
		RenderPassInfo renderPassInfo;
		renderPassInfo.RenderTextureInfo.Width = 1;
		renderPassInfo.RenderTextureInfo.Height = 1;
		renderPassInfo.RenderTextureInfo.HasDepth = true;
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("object", PixelFormat::PF_RGBAI, PixelFormat::PF_R32G32B32A32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UInt, false));
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R16G16B16A16Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		renderPassInfo.m_Position = RenderPassPosition::RP_Start;
		renderPassInfo.m_LoadOp = RenderPassLoadOp::OP_Clear;
		m_DummyRenderPass = pDevice->CreateRenderPass(std::move(renderPassInfo));

		RenderPassInfo ssaoRenderPassInfo;
		ssaoRenderPassInfo.RenderTextureInfo.Width = 1;
		ssaoRenderPassInfo.RenderTextureInfo.Height = 1;
		ssaoRenderPassInfo.RenderTextureInfo.HasDepth = false;
		ssaoRenderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("AO", PixelFormat::PF_R, PixelFormat::PF_R32Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		m_DummySSAORenderPass = pDevice->CreateRenderPass(std::move(ssaoRenderPassInfo));

		PostProcess displayCopyPP;
		displayCopyPP.m_Name = "Initial Display Copy";
		displayCopyPP.m_Priority = INT32_MAX;
		displayCopyPP.m_Callback = [this](GraphicsDevice* pDevice, CameraRef camera, size_t,
			CommandBufferHandle commandBuffer, size_t, RenderPassHandle renderPass, DescriptorSetHandle) {
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());

			const glm::uvec2& resolution = camera.GetResolution();
			pDevice->BeginRenderPass(commandBuffer, renderPass);
			pDevice->BeginPipeline(commandBuffer, m_DisplayCopyPipeline);
			pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
			pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
			pDevice->BindDescriptorSets(commandBuffer, m_DisplayCopyPipeline, { uniqueCameraData.m_ColorSamplerSets[m_CurrentFrameIndex] });
			pDevice->DrawQuad(commandBuffer);
			pDevice->EndPipeline(commandBuffer);
			pDevice->EndRenderPass(commandBuffer);
			return true;
		};

		AddPostProcess(std::move(displayCopyPP));

		PostProcess ssaoPP;
		ssaoPP.m_Name = "SSAO";
		ssaoPP.m_Priority = 0;
		ssaoPP.m_Callback = [this](GraphicsDevice* pDevice, CameraRef camera, size_t,
			CommandBufferHandle commandBuffer, size_t, RenderPassHandle renderPass, DescriptorSetHandle colorSet)
		{
			if (!m_GlobalSSAOSetting.m_Enabled) return false;
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			const DescriptorSetHandle& ssaoSamplersSet = uniqueCameraData.m_SSAOPostSamplersSets[m_CurrentFrameIndex];

			float aoSettings[2] = {
				m_GlobalSSAOSetting.m_Magnitude,
				m_GlobalSSAOSetting.m_Contrast,
			};

			const glm::uvec2& resolution = camera.GetResolution();
			pDevice->BeginRenderPass(commandBuffer, renderPass);
			pDevice->BeginPipeline(commandBuffer, m_SSAOPostPassPipeline);
			pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
			pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
			pDevice->BindDescriptorSets(commandBuffer, m_SSAOPostPassPipeline, { colorSet, ssaoSamplersSet });
			pDevice->PushConstants(commandBuffer, m_SSAOPostPassPipeline, 0, sizeof(aoSettings), aoSettings, STF_Fragment);
			pDevice->DrawQuad(commandBuffer);
			pDevice->EndPipeline(commandBuffer);
			pDevice->EndRenderPass(commandBuffer);
			return true;
		};

		AddPostProcess(std::move(ssaoPP));

		PostProcess attachmentVisPP;
		attachmentVisPP.m_Name = "Camera Attachment Visualizer";
		attachmentVisPP.m_Priority = INT32_MIN;
		attachmentVisPP.m_Callback = [this](GraphicsDevice* pDevice, CameraRef camera, size_t,
			CommandBufferHandle commandBuffer, size_t, RenderPassHandle renderPass, DescriptorSetHandle colorSet)
		{
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());

			DescriptorSetHandle previewSamplerSet = NULL;
			PipelineHandle previewPipeline = NULL;

			switch (uniqueCameraData.m_VisualizedAttachment)
			{
			case CameraAttachment::ObjectID:
				previewPipeline = m_VisualizeObjectIDPipeline;
				previewSamplerSet = uniqueCameraData.m_ObjectIDSamplerSets[m_CurrentFrameIndex];
				break;
			case CameraAttachment::Color:
				previewPipeline = m_DisplayCopyPipeline;
				previewSamplerSet = uniqueCameraData.m_ColorSamplerSets[m_CurrentFrameIndex];
				break;
			case CameraAttachment::Normal:
				previewPipeline = m_DisplayCopyPipeline;
				previewSamplerSet = uniqueCameraData.m_NormalSamplerSets[m_CurrentFrameIndex];
				break;
			case CameraAttachment::AO:
				previewPipeline = m_VisualizeSSAOPipeline;
				previewSamplerSet = uniqueCameraData.m_AOSamplerSets[m_CurrentFrameIndex];
				break;
			case CameraAttachment::Depth:
				previewPipeline = m_VisualizeDepthPipeline;
				previewSamplerSet = uniqueCameraData.m_DepthSamplerSets[m_CurrentFrameIndex];
				break;
			default:
				return false;
			}

			const glm::uvec2& resolution = camera.GetResolution();
			pDevice->BeginRenderPass(commandBuffer, renderPass);
			pDevice->BeginPipeline(commandBuffer, previewPipeline);
			pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
			pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
			if (uniqueCameraData.m_VisualizedAttachment == CameraAttachment::Depth)
			{
				float constants[2] = {
					camera.GetNear(),
					camera.GetFar(),
				};
				pDevice->PushConstants(commandBuffer, previewPipeline, 0, sizeof(constants), constants, STF_Fragment);
			}
			pDevice->BindDescriptorSets(commandBuffer, previewPipeline, { previewSamplerSet });
			pDevice->DrawQuad(commandBuffer);
			pDevice->EndPipeline(commandBuffer);
			pDevice->EndRenderPass(commandBuffer);
			return true;
		};

		AddPostProcess(std::move(attachmentVisPP));

		PostProcess debugOverlayPP;
		debugOverlayPP.m_Name = "Debug Overlays";
		debugOverlayPP.m_Priority = INT32_MIN;
		debugOverlayPP.m_Callback = [this](GraphicsDevice* pDevice, CameraRef camera, size_t cameraIndex,
			CommandBufferHandle commandBuffer, size_t, RenderPassHandle renderPass, DescriptorSetHandle colorSet)
		{
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			if (!m_DebugOverlayBits.HasAnySet() && !uniqueCameraData.m_DebugOverlayBits.HasAnySet()) return false;

			const bool shadowAtlas = m_DebugOverlayBits.IsSet(DebugOverlayBitIndices::ShadowAtlas) ||
				uniqueCameraData.m_DebugOverlayBits.IsSet(DebugOverlayBitIndices::ShadowAtlas);
			const bool lightComplexity = m_DebugOverlayBits.IsSet(DebugOverlayBitIndices::LightComplexity) ||
				uniqueCameraData.m_DebugOverlayBits.IsSet(DebugOverlayBitIndices::LightComplexity);

			const glm::uvec2& resolution = camera.GetResolution();
			pDevice->BeginRenderPass(commandBuffer, renderPass);

			/* Copy the display first so we can render overlays on top */
			pDevice->BeginPipeline(commandBuffer, m_DisplayCopyPipeline);
			pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
			pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
			pDevice->BindDescriptorSets(commandBuffer, m_DisplayCopyPipeline, { colorSet });
			pDevice->DrawQuad(commandBuffer);
			pDevice->EndPipeline(commandBuffer);

			if (lightComplexity)
			{
				pDevice->BeginPipeline(commandBuffer, m_VisualizeLightComplexityPipeline);
				pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
				pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);

				LightComplexityConstants constants;
				constants.zNear = camera.GetNear();
				constants.zFar = camera.GetFar();
				constants.Resolution = camera.GetResolution();
				constants.GridSize = glm::uvec4(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES, 0.0f);

				pDevice->PushConstants(commandBuffer, m_VisualizeLightComplexityPipeline, 0, sizeof(LightComplexityConstants), &constants, STF_Fragment);
				pDevice->BindDescriptorSets(commandBuffer, m_VisualizeLightComplexityPipeline,
					{ uniqueCameraData.m_DepthSamplerSets[m_CurrentFrameIndex], uniqueCameraData.m_LightGridSets[m_CurrentFrameIndex] });
				pDevice->DrawQuad(commandBuffer);
				pDevice->EndPipeline(commandBuffer);
			}

			if (shadowAtlas)
			{
				const uint32_t size = resolution.y/2;

				pDevice->BeginPipeline(commandBuffer, m_VisualizeDepthPipeline);
				pDevice->SetViewport(commandBuffer, 0.0f, float(resolution.y - size), float(size), float(size));
				pDevice->SetScissor(commandBuffer, 0, resolution.y - size, size, size);
				float constants[2] = {
					camera.GetNear(),
					camera.GetFar(),
				};
				pDevice->PushConstants(commandBuffer, m_VisualizeDepthPipeline, 0, sizeof(constants), constants, STF_Fragment);
				pDevice->BindDescriptorSets(commandBuffer, m_VisualizeDepthPipeline, { m_ShadowAtlasSamplerSets[m_CurrentFrameIndex]});
				pDevice->DrawQuad(commandBuffer);
				pDevice->EndPipeline(commandBuffer);
			}

			pDevice->EndRenderPass(commandBuffer);
			return true;
		};

		AddPostProcess(std::move(debugOverlayPP));
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
		const UUID ssaoPostpassPipeline = settings.Value<uint64_t>("SSAO Postpass");
		const UUID ssaoVisualizerPipeline = settings.Value<uint64_t>("SSAO Visualizer");
		const UUID objectIDVisualizerPipeline = settings.Value<uint64_t>("ObjectID Visualizer");
		const UUID depthVisualizerPipeline = settings.Value<uint64_t>("Depth Visualizer");
		const UUID lightComplexityVisualizerPipeline = settings.Value<uint64_t>("Light Complexity Visualizer");

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
			pDevice->AcquireNextSwapchainImage(m_Swapchain, &m_CurrentFrameIndex, m_ImageAvailableSemaphores[m_CurrentSemaphoreIndex]);
			waitSemaphores.push_back(m_ImageAvailableSemaphores[m_CurrentSemaphoreIndex]);
			signalSemaphores.push_back(m_RenderingFinishedSemaphores[m_CurrentSemaphoreIndex]);
		}

		for (auto& injectedDataPass : m_InjectedDataPasses)
			injectedDataPass(pDevice, this);

		PrepareDataPass();

		/* Make sure every camera has a render pass */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];
			UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			const auto& resolution = camera.GetResolution();

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

		for (auto& injectedPrePass : m_InjectedPreRenderPasses)
			injectedPrePass(pDevice, m_FrameCommandBuffers[m_CurrentFrameIndex], m_CurrentFrameIndex);

		const GPUTextureAtlas& shadowAtlas = GetGPUTextureAtlas(m_ShadowAtlasses[m_CurrentFrameIndex]);
		/* Wait for shadow rendering to finish */

		ImageBarrier shadowBarrier;
		shadowBarrier.m_Texture = shadowAtlas.GetTexture();
		shadowBarrier.m_SrcAccessMask = AF_DepthStencilAttachmentWrite;
		shadowBarrier.m_DstAccessMask = AF_ShaderRead;
		pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { shadowBarrier },
			PipelineStageFlagBits::PST_LateFragmentTests, PipelineStageFlagBits::PST_FragmentShader);

		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera " + std::to_string(i) + " forward pass"};
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			const BufferHandle& lightIndexSSBO = uniqueCameraData.m_LightIndexSSBOs[m_CurrentFrameIndex];
			const BufferHandle& lightGridSSBO = uniqueCameraData.m_LightGridSSBOs[m_CurrentFrameIndex];

			/* Wait for light culling to finish */
			const std::vector<BufferBarrier> lightCullingBarriers = {
				{
					lightIndexSSBO,
					AF_ShaderWrite,
					AF_ShaderRead
				},
				{
					lightGridSSBO,
					AF_ShaderWrite,
					AF_ShaderRead
				},
			};
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], lightCullingBarriers, {},
				PipelineStageFlagBits::PST_ComputeShader, PipelineStageFlagBits::PST_FragmentShader);

			/* Draw objects */
			const RenderPassHandle& renderPass = uniqueCameraData.m_RenderPasses[m_CurrentFrameIndex];
			pDevice->SetRenderPassClear(renderPass, camera.GetClearColor());
			pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], renderPass);
			SkyboxPass(m_FrameCommandBuffers[m_CurrentFrameIndex], static_cast<uint32_t>(i));
			DynamicObjectsPass(m_FrameCommandBuffers[m_CurrentFrameIndex], static_cast<uint32_t>(i));

			if (m_LineVertexCount)
			{
				uint32_t cameraIndex = static_cast<uint32_t>(i);
				pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineRenderPipeline);
				pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineRenderPipeline, { m_GlobalLineRenderSet });
				pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineRenderPipeline, 0, sizeof(uint32_t), &cameraIndex, STF_Vertex);
				pDevice->DrawMesh(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineMeshes[m_CurrentFrameIndex]);
				pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			}
			pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}

		/* Picking */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera " + std::to_string(i) + " picking" };
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			if (uniqueCameraData.m_Picks.empty()) continue;

			const RenderPassHandle& renderPass = uniqueCameraData.m_RenderPasses[m_CurrentFrameIndex];
			const RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			const TextureHandle objectID = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			const TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 2);
			const TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 3);

			/* Wait for rendering to finish */
			const std::vector<ImageBarrier> imageBarriers = {
				{
					objectID,
					AF_ColorAttachmentWrite,
					AF_ShaderRead
				},
				{
					normals,
					AF_ColorAttachmentWrite,
					AF_ShaderRead
				},
			};
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, imageBarriers,
				PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_ComputeShader);

			ImageBarrier depthBarrier{
				depth,
				AF_DepthStencilAttachmentWrite,
				AF_ShaderRead
			};
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { depthBarrier },
				PipelineStageFlagBits::PST_LateFragmentTests, PipelineStageFlagBits::PST_ComputeShader);

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
			pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], m_PickingPipeline,
				0, sizeof(PickingConstants), &constants, STF_Compute);
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
			BufferBarrier pickingBarrier;
			pickingBarrier.m_Buffer = pickResults;
			pickingBarrier.m_SrcAccessMask = AF_ShaderWrite;
			pickingBarrier.m_DstAccessMask = AF_None;
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], { pickingBarrier }, {},
				PipelineStageFlagBits::PST_ComputeShader, PipelineStageFlagBits::PST_FragmentShader);

			/* Draw late objects */
			const RenderPassHandle& renderPass = uniqueCameraData.m_LateRenderPasses[m_CurrentFrameIndex];
			pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], renderPass);
			DynamicLateObjectsPass(m_FrameCommandBuffers[m_CurrentFrameIndex], static_cast<uint32_t>(i));
			pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}

		if (m_GlobalSSAOSetting.m_Enabled)
		{
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
				ImageBarrier normalBarrier{
					normals,
					AF_ColorAttachmentWrite,
					AF_ShaderRead
				};
				pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { normalBarrier },
					PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);

				ImageBarrier depthBarrier{
					depth,
					AF_DepthStencilAttachmentWrite,
					AF_ShaderRead
				};
				pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { depthBarrier },
					PipelineStageFlagBits::PST_LateFragmentTests, PipelineStageFlagBits::PST_FragmentShader);

				/* SSAO pass */
				pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], ssaoRenderPass);
				pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SSAOPipeline);
				pDevice->SetViewport(m_FrameCommandBuffers[m_CurrentFrameIndex], 0.0f, 0.0f, float(resolution.x), float(resolution.y));
				pDevice->SetScissor(m_FrameCommandBuffers[m_CurrentFrameIndex], 0, 0, resolution.x, resolution.y);
				pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SSAOPipeline, { m_SSAOCameraSet, m_GlobalSampleDomeSet, ssaoSamplersSet, m_NoiseSamplerSet });
				pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SSAOPipeline, 0, sizeof(SSAOConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
				pDevice->DrawQuad(m_FrameCommandBuffers[m_CurrentFrameIndex]);
				pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
				pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			}
		}

		std::vector<ImageBarrier> colorBarriers(m_ActiveCameras.size());
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(uniqueCameraData.m_RenderPasses[m_CurrentFrameIndex]);
			colorBarriers[i].m_Texture = pDevice->GetRenderTextureAttachment(renderTexture, 1);
			colorBarriers[i].m_SrcAccessMask = AF_ColorAttachmentWrite;
			colorBarriers[i].m_DstAccessMask = AF_ShaderRead;
		}

		if (!colorBarriers.empty())
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, colorBarriers,
				PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);

		/* Post processing */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera " + std::to_string(i) + " Post Processing" };
			CameraRef camera = m_ActiveCameras[i];
			UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			PostProcessPass& postProcessPass = uniqueCameraData.m_PostProcessPasses[m_CurrentFrameIndex];

			for (const PostProcess& pp : m_PostProcesses)
			{
				ProfileSample profile{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Camera Post Process: " + pp.m_Name };
				const bool rendered = pp.m_Callback(pDevice, camera, i, m_FrameCommandBuffers[m_CurrentFrameIndex], m_CurrentFrameIndex,
					postProcessPass.m_BackBufferPass, postProcessPass.m_FrontDescriptor);
				if (rendered)
				{
					RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(postProcessPass.m_BackBufferPass);
					TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 0);

					ImageBarrier colorBarrier;
					colorBarrier.m_Texture = color;
					colorBarrier.m_SrcAccessMask = AF_ColorAttachmentWrite;
					colorBarrier.m_DstAccessMask = AF_ShaderRead;
					pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { colorBarrier },
						PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);
					postProcessPass.Swap();
				}
			}
		}

		pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], m_FinalFrameColorPasses[m_CurrentFrameIndex]);
		pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_DisplayCopyPipeline);
		for (size_t i = 0; i < m_OutputCameras.size(); ++i)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Render output from camera " + std::to_string(i) };
			CameraRef camera = m_OutputCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_DisplayCopyPipeline,
				{ uniqueCameraData.m_PostProcessPasses[m_CurrentFrameIndex].m_FrontDescriptor });
			pDevice->DrawQuad(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}
		pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);

		if (m_Swapchain)
		{
			ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::Draw: Render to swap chain image" };
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[m_CurrentFrameIndex]);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 0);

			ImageBarrier colorBarrier;
			colorBarrier.m_Texture = color;
			colorBarrier.m_SrcAccessMask = AF_ColorAttachmentWrite;
			colorBarrier.m_DstAccessMask = AF_ShaderRead;
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { colorBarrier },
				PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);

			pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SwapchainPasses[m_CurrentSemaphoreIndex]);
			pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], m_DisplayCopyPipeline);
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], m_DisplayCopyPipeline,
				{ m_FinalFrameColorSets[m_CurrentFrameIndex] });
			pDevice->DrawQuad(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			for (auto& injectedSubpass : m_InjectedSwapchainSubpasses)
				injectedSubpass(pDevice, m_SwapchainPasses[m_CurrentSemaphoreIndex], m_FrameCommandBuffers[m_CurrentFrameIndex]);
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
		settings.RegisterAssetReference<PipelineData>("SSAO Postpass", 49);
		settings.RegisterAssetReference<PipelineData>("SSAO Visualizer", 51);
		settings.RegisterAssetReference<PipelineData>("ObjectID Visualizer", 50);
		settings.RegisterAssetReference<PipelineData>("Depth Visualizer", 52);
		settings.RegisterAssetReference<PipelineData>("Light Complexity Visualizer", 54);
	}

	void GloryRendererModule::Preload()
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;
		CheckCachedPipelines(pDevice);
	}

	size_t GloryRendererModule::DefaultAttachmenmtIndex() const
	{
		return 5;
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
		const PostProcessPass& pp = uniqueCameraData.m_PostProcessPasses[m_CurrentFrameIndex];
		if (!renderPass || !ssaoRenderPass) return NULL;
		if (index == 5)
		{
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(pp.m_FrontBufferPass);
			return pDevice->GetRenderTextureAttachment(renderTexture, 0);
		}
		if (index >= 4)
		{
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(ssaoRenderPass);
			return pDevice->GetRenderTextureAttachment(renderTexture, index-4);
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

	void GloryRendererModule::VisualizeAttachment(CameraRef camera, size_t index)
	{
		if (index >= CameraAttachment::Count) return;
		UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		uniqueCameraData.m_VisualizedAttachment = CameraAttachment(index);
	}

	size_t GloryRendererModule::DebugOverlayCount() const
	{
		return DebugOverlayNameCount;
	}

	std::string_view GloryRendererModule::DebugOverlayName(size_t index) const
	{
		return DebugOverlayNames[index];
	}

	void GloryRendererModule::SetDebugOverlayEnabled(CameraRef camera, size_t index, bool enabled)
	{
		if (camera == NULL)
		{
			m_DebugOverlayBits.Set(index, enabled);
			return;
		}

		UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		uniqueCameraData.m_DebugOverlayBits.Set(index, enabled);
	}

	bool GloryRendererModule::DebugOverlayEnabled(CameraRef camera, size_t index) const
	{
		if (camera == NULL)
			return m_DebugOverlayBits.IsSet(index);

		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		return uniqueCameraData.m_DebugOverlayBits.IsSet(index);
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
				m_ShadowAtlasSamplerSets[i] = CreateSamplerDescriptorSet(pDevice, 1, { texture }, m_ShadowAtlasSamplerSetLayout);
			}

			RenderPassInfo postProcessPassInfo;
			postProcessPassInfo.RenderTextureInfo.HasDepth = false;
			postProcessPassInfo.RenderTextureInfo.HasStencil = false;
			postProcessPassInfo.RenderTextureInfo.EnableDepthStencilSampling = false;
			postProcessPassInfo.RenderTextureInfo.Width = m_Resolution.x;
			postProcessPassInfo.RenderTextureInfo.Height = m_Resolution.y;
			postProcessPassInfo.RenderTextureInfo.Attachments.push_back(
				Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Unorm,
					Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float)
			);
			if (!m_FinalFrameColorPasses[i])
				m_FinalFrameColorPasses[i] = pDevice->CreateRenderPass(std::move(postProcessPassInfo));

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

	RenderPassHandle GloryRendererModule::GetSwapchainPass() const
	{
		return !m_SwapchainPasses.empty() ? m_SwapchainPasses[0] : NULL;
	}

	RenderPassHandle GloryRendererModule::GetDummyPostProcessPass() const
	{
		return m_FinalFrameColorPasses[0];
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

		RenderConstants constants;
		constants.m_CameraIndex = static_cast<uint32_t>(cameraIndex);
		constants.m_LightCount = m_FrameData.ActiveLights.count();
		constants.m_GridSize = glm::uvec4(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES, 0.0f);
		CameraRef camera = m_ActiveCameras[cameraIndex];
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		const DescriptorSetHandle lightSet = uniqueCameraData.m_LightSets[m_CurrentFrameIndex];
		const LayerMask& cameraMask = camera.GetLayerMask();

		for (auto pipelineID : m_PipelineOrder)
		{
			auto iter = std::find_if(batches.begin(), batches.end(),
				[pipelineID](const PipelineBatch& batch) {
					return batch.m_PipelineID == pipelineID;
				});

			if (iter == batches.end()) continue;
			const size_t batchIndex = iter - batches.begin();

			if (batchIndex >= batchDatas.size()) continue;
			const PipelineBatch& pipelineRenderData = *iter;
			const PipelineBatchData& batchData = batchDatas.at(batchIndex);

			PipelineData* pPipelineData = pipelines.GetPipelineData(pipelineRenderData.m_PipelineID);
			if (!pPipelineData) continue;
			pDevice->BeginPipeline(commandBuffer, batchData.m_Pipeline);
			if (viewport.z > 0.0f && viewport.w > 0.0f)
			{
				pDevice->SetViewport(commandBuffer, viewport.x, viewport.y, viewport.z, viewport.w);
				pDevice->SetScissor(commandBuffer, int(viewport.x), int(viewport.y), viewport.z, viewport.w);
			}

			pDevice->BindDescriptorSets(commandBuffer, batchData.m_Pipeline,
				{ globalRenderSet, batchData.m_ObjectDataSet, m_GlobalLightSet, lightSet, batchData.m_MaterialSet });

			if (shadowsSet)
				pDevice->BindDescriptorSets(commandBuffer, batchData.m_Pipeline, { shadowsSet }, 5);

			if (pPipelineData->BlendEnabled())
			{
				/* Sort objects based on distance from camera */
				std::set<OrderedObject> renderOrder;
				uint32_t objectIndex = 0;
				for (UUID uniqueMeshID : pipelineRenderData.m_UniqueMeshOrder)
				{
					const PipelineMeshBatch& meshBatch = pipelineRenderData.m_Meshes.at(uniqueMeshID);
					Resource* pMeshResource = assets.FindResource(meshBatch.m_Mesh);
					if (!pMeshResource) continue;
					MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);

					const BoundingBox& bounds = pMeshData->GetBoundingBox();
					std::vector<glm::vec4> points(8);

					points[0] = bounds.m_Center + glm::vec4(-bounds.m_HalfExtends.x, bounds.m_HalfExtends.y, -bounds.m_HalfExtends.z, 1.0f);
					points[1] = bounds.m_Center + glm::vec4(bounds.m_HalfExtends.x, bounds.m_HalfExtends.y, -bounds.m_HalfExtends.z, 1.0f);
					points[2] = bounds.m_Center + glm::vec4(bounds.m_HalfExtends.x, bounds.m_HalfExtends.y, bounds.m_HalfExtends.z, 1.0f);
					points[3] = bounds.m_Center + glm::vec4(-bounds.m_HalfExtends.x, bounds.m_HalfExtends.y, bounds.m_HalfExtends.z, 1.0f);
					points[4] = bounds.m_Center + glm::vec4(-bounds.m_HalfExtends.x, -bounds.m_HalfExtends.y, -bounds.m_HalfExtends.z, 1.0f);
					points[5] = bounds.m_Center + glm::vec4(bounds.m_HalfExtends.x, -bounds.m_HalfExtends.y, -bounds.m_HalfExtends.z, 1.0f);
					points[6] = bounds.m_Center + glm::vec4(bounds.m_HalfExtends.x, -bounds.m_HalfExtends.y, bounds.m_HalfExtends.z, 1.0f);
					points[7] = bounds.m_Center + glm::vec4(-bounds.m_HalfExtends.x, -bounds.m_HalfExtends.y, bounds.m_HalfExtends.z, 1.0f);

					for (size_t meshObjectIndex = 0; meshObjectIndex < meshBatch.m_Worlds.size(); ++meshObjectIndex)
					{
						const auto& world = meshBatch.m_Worlds[meshObjectIndex];

						const uint32_t currentObject = objectIndex;
						++objectIndex;

						float minDistance = FLT_MAX;
						for (size_t pointIndex = 0; pointIndex < points.size(); ++pointIndex)
						{
							const glm::vec4 point = world*points[pointIndex];
							const glm::vec4 cameraPos = camera.GetViewInverse()[3];
							const float distance = glm::distance(glm::vec3(point), glm::vec3(cameraPos));
							if (distance > minDistance) continue;
							minDistance = distance;
						}
						renderOrder.insert(OrderedObject{ minDistance, uniqueMeshID, uint32_t(meshObjectIndex), currentObject });
					}
				}

				for (auto& orderedObject : renderOrder)
				{
					const PipelineMeshBatch& meshBatch = pipelineRenderData.m_Meshes.at(orderedObject.meshID);
					Resource* pMeshResource = assets.FindResource(meshBatch.m_Mesh);
					if (!pMeshResource) continue;
					MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
					MeshHandle mesh = pDevice->AcquireCachedMesh(pMeshData);
					if (!mesh) continue;

					if (cameraMask != 0 && meshBatch.m_LayerMasks[orderedObject.meshObjectIndex] != 0 &&
						(cameraMask & meshBatch.m_LayerMasks[orderedObject.meshObjectIndex]) == 0) continue;

					const auto& ids = meshBatch.m_ObjectIDs[orderedObject.meshObjectIndex];
					constants.m_ObjectID = ids.second;
					constants.m_SceneID = ids.first;
					constants.m_ObjectDataIndex = orderedObject.objectIndex;
					constants.m_MaterialIndex = meshBatch.m_MaterialIndices[orderedObject.meshObjectIndex];

					pDevice->PushConstants(commandBuffer, batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
					if (!batchData.m_TextureSets.empty())
						pDevice->BindDescriptorSets(commandBuffer, batchData.m_Pipeline, { batchData.m_TextureSets[constants.m_MaterialIndex] }, 6);
					pDevice->DrawMesh(commandBuffer, mesh);
				}
				pDevice->EndPipeline(commandBuffer);
				return;
			}

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

					const auto& ids = meshBatch.m_ObjectIDs[i];
					constants.m_ObjectID = ids.second;
					constants.m_SceneID = ids.first;
					constants.m_ObjectDataIndex = currentObject;
					constants.m_MaterialIndex = meshBatch.m_MaterialIndices[i];

					pDevice->PushConstants(commandBuffer, batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
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
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		CheckCachedPipelines(pDevice);

		PrepareCameras();

		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			CameraRef camera = m_ActiveCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			if (std::find(m_DirtyCameraPerspectives.begin(), m_DirtyCameraPerspectives.end(), camera) != m_DirtyCameraPerspectives.end())
				GenerateClusterSSBO(i, pDevice, camera, uniqueCameraData.m_ClusterSet);
		}
		m_DirtyCameraPerspectives.clear();

		for (size_t i = 0; i < m_FrameData.Picking.size(); ++i)
		{
			const glm::ivec2& pos = m_FrameData.Picking[i].first;
			const UUID cameraID = m_FrameData.Picking[i].second;
			UniqueCameraData& cameraData = m_UniqueCameraDatas.at(cameraID);
			cameraData.m_Picks.emplace_back(pos);
		}

		if (m_LightCameraDatas->size() < m_FrameData.LightViews.count())
		{
			m_LightCameraDatas.resize(m_FrameData.LightViews.count());
			m_LightSpaceTransforms.resize(m_FrameData.LightViews.count());
		}
		for (size_t i = 0; i < m_FrameData.LightViews.count(); ++i)
		{
			glm::mat4 projection = m_FrameData.LightProjections[i];
			FixProjection(projection, pDevice);

			if (m_LightCameraDatas.m_Data[i].m_Projection != projection)
			{
				m_LightCameraDatas.m_Data[i].m_Projection = projection;
				m_LightCameraDatas.SetDirty(i);
			}
			if (m_LightCameraDatas.m_Data[i].m_View != m_FrameData.LightViews[i])
			{
				m_LightCameraDatas.m_Data[i].m_View = m_FrameData.LightViews[i];
				m_LightCameraDatas.SetDirty(i);
			}

			const glm::mat4 lightSpace = projection*m_FrameData.LightViews[i];
			if (m_LightSpaceTransforms.m_Data[i] != lightSpace)
			{
				m_LightSpaceTransforms.m_Data[i] = lightSpace;
				m_LightSpaceTransforms.SetDirty(i);
			}
		}
		if (m_LightCameraDatas)
		{
			const size_t dirtySize = m_LightCameraDatas.DirtySize();
			pDevice->AssignBuffer(m_LightCameraDatasBuffer, m_LightCameraDatas.DirtyStart(),
				m_LightCameraDatas.m_DirtyRange.first*sizeof(PerCameraData), dirtySize*sizeof(PerCameraData));
		}

		if (m_LightSpaceTransforms)
		{
			const size_t dirtySize = m_LightSpaceTransforms.DirtySize();
			pDevice->AssignBuffer(m_LightSpaceTransformsSSBO, m_LightSpaceTransforms.DirtyStart(),
				m_LightSpaceTransforms.m_DirtyRange.first*sizeof(glm::mat4), dirtySize*sizeof(glm::mat4));
		}

		/* Prepare shadow resolutions and atlas coords */
		const uint32_t sliceSteps = NUM_DEPTH_SLICES/m_MaxShadowLODs;

		GPUTextureAtlas& shadowAtlas = GetGPUTextureAtlas(m_ShadowAtlasses[m_CurrentFrameIndex]);
		shadowAtlas.ReleaseAllChunks();
		for (size_t i = 0; i < m_FrameData.ActiveLights.count(); ++i)
		{
			auto& lightData = m_FrameData.ActiveLights[i];
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
			FixShadowCoords(lightData.shadowCoords, pDevice);
		}

		/* Update light data */
		pDevice->AssignBuffer(m_LightsSSBO, m_FrameData.ActiveLights.data(), 0, MAX_LIGHTS*sizeof(LightData));

		PrepareBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData);
		PrepareBatches(m_DynamicLatePipelineRenderDatas, m_DynamicLateBatchData);
		PrepareLineMesh(pDevice);
		PrepareSkybox(pDevice);
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

			if (pipelineBatch.m_UniqueMeshOrder.empty()) continue;

			size_t objectCount = 0;
			for (const UUID meshID : pipelineBatch.m_UniqueMeshOrder)
			{
				const PipelineMeshBatch& meshBatch = pipelineBatch.m_Meshes.at(meshID);
				if (batchData.m_Worlds->size() < objectCount + meshBatch.m_Worlds.size())
					batchData.m_Worlds.resize(objectCount + meshBatch.m_Worlds.size());

				for (size_t i = 0; i < meshBatch.m_Worlds.size(); ++i)
				{
					if (std::memcmp(&batchData.m_Worlds.m_Data[objectCount + i],
						&meshBatch.m_Worlds[i], sizeof(glm::mat4)) != 0)
					{
						std::memcpy(&batchData.m_Worlds.m_Data[objectCount + i],
							&meshBatch.m_Worlds[i], sizeof(glm::mat4));
						batchData.m_Worlds.SetDirty(objectCount + i);
					}
				}
				objectCount += meshBatch.m_Worlds.size();
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
			if (batchData.m_LastFrameUniqueMaterials.size() < pipelineBatch.m_UniqueMaterials.size())
				batchData.m_LastFrameUniqueMaterials.resize(pipelineBatch.m_UniqueMaterials.size(), NULL);

			if (!batchData.m_TextureSetLayout && textureCount > 0)
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
			}
			if (textureCount > 0 && batchData.m_TextureSets.size() < pipelineBatch.m_UniqueMaterials.size())
				batchData.m_TextureSets.resize(pipelineBatch.m_UniqueMaterials.size(), nullptr);

			for (size_t i = 0; i < pipelineBatch.m_UniqueMaterials.size(); ++i)
			{
				const UUID materialID = pipelineBatch.m_UniqueMaterials[i];
				const bool differentMaterial = batchData.m_LastFrameUniqueMaterials[i] != materialID;
				batchData.m_LastFrameUniqueMaterials[i] = materialID;

				MaterialData* pMaterialData = materials.GetMaterial(materialID);
				if (!pMaterialData) continue;
				const auto& buffer = pMaterialData->GetBufferReference();
				if (std::memcmp(&batchData.m_MaterialDatas.m_Data[i*finalPropertyDataSize], buffer.data(), buffer.size()) != 0)
				{
					std::memcpy(&batchData.m_MaterialDatas.m_Data[i*finalPropertyDataSize], buffer.data(), buffer.size());
					batchData.m_MaterialDatas.SetDirty(i);
					batchData.m_MaterialDatas.SetDirty(i + finalPropertyDataSize - 1);
				}
				const uint32_t textureBits = pMaterialData->TextureSetBits();
				if (textureCount && batchData.m_TextureBits.m_Data[i] != textureBits)
				{
					batchData.m_TextureBits.m_Data[i] = textureBits;
					batchData.m_TextureBits.SetDirty(i);
				}

				if (textureCount == 0)
				{
					pMaterialData->SetDirty(false);
					continue;
				}

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
							setInfo.m_Samplers[j].m_TextureHandle = NULL;
							continue;
						}
						TextureData* pTexture = static_cast<TextureData*>(pResource);
						setInfo.m_Samplers[j].m_TextureHandle = pDevice->AcquireCachedTexture(pTexture);
					}
					batchData.m_TextureSets[i] = pDevice->CreateDescriptorSet(std::move(setInfo));
					pMaterialData->SetDirty(false);
					continue;
				}

				std::vector<TextureData*> textures(textureCount);
				bool texturesDirty = false;
				for (size_t j = 0; j < textureCount; ++j)
				{
					const UUID textureID = pMaterialData->GetResourceUUIDPointer(j)->AssetUUID();
					Resource* pResource = assets.FindResource(textureID);
					if (!pResource)
					{
						textures[j] = NULL;
						continue;
					}
					textures[j] = static_cast<TextureData*>(pResource);
					ImageData* pImage = textures[j]->GetImageData(&assets);
					texturesDirty |= textures[j]->IsDirty() || pImage ? pImage->IsDirty() : false;
				}

				if (pMaterialData->IsDirty() || differentMaterial || texturesDirty)
				{
					DescriptorSetUpdateInfo dsUpdateInfo;
					dsUpdateInfo.m_Samplers.resize(textureCount);
					for (size_t j = 0; j < textureCount; ++j)
					{
						TextureData* pTexture = textures[j];
						dsUpdateInfo.m_Samplers[j].m_DescriptorIndex = j;
						dsUpdateInfo.m_Samplers[j].m_TextureHandle = pDevice->AcquireCachedTexture(pTexture);
					}
					pDevice->UpdateDescriptorSet(batchData.m_TextureSets[i], dsUpdateInfo);
				}
				pMaterialData->SetDirty(false);
			}

			if (!batchData.m_WorldsBuffer)
			{
				batchData.m_WorldsBuffer = pDevice->CreateBuffer(batchData.m_Worlds->size()*sizeof(glm::mat4), BT_Storage, BF_Write);
				batchData.m_Worlds.SetDirty();
			}
			if (pDevice->BufferSize(batchData.m_WorldsBuffer) < batchData.m_Worlds.TotalByteSize())
				pDevice->ResizeBuffer(batchData.m_WorldsBuffer, batchData.m_Worlds.TotalByteSize());
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
			if (pDevice->BufferSize(batchData.m_MaterialsBuffer) < batchData.m_MaterialDatas.TotalByteSize())
				pDevice->ResizeBuffer(batchData.m_MaterialsBuffer, batchData.m_MaterialDatas.TotalByteSize());
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
			if (textureCount && pDevice->BufferSize(batchData.m_TextureBitsBuffer) < batchData.m_TextureBits.TotalByteSize())
				pDevice->ResizeBuffer(batchData.m_TextureBitsBuffer, batchData.m_TextureBits.TotalByteSize());
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
				setInfo.m_Buffers[0].m_Size = batchData.m_Worlds->size()*sizeof(glm::mat4);
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

			if (batchData.m_Worlds.SizeDirty())
			{
				DescriptorSetUpdateInfo dsWrite;
				dsWrite.m_Buffers.resize(1);
				dsWrite.m_Buffers[0].m_BufferHandle = batchData.m_WorldsBuffer;
				dsWrite.m_Buffers[0].m_DescriptorIndex = 0;
				dsWrite.m_Buffers[0].m_Offset = 0;
				dsWrite.m_Buffers[0].m_Size = batchData.m_Worlds->size()*sizeof(glm::mat4);
				pDevice->UpdateDescriptorSet(batchData.m_ObjectDataSet, dsWrite);
			}
			if (batchData.m_MaterialDatas.SizeDirty() || batchData.m_TextureBits.SizeDirty())
			{
				DescriptorSetUpdateInfo dsWrite;
				dsWrite.m_Buffers.resize(textureCount > 0 ? 2 : 1);
				dsWrite.m_Buffers[0].m_BufferHandle = batchData.m_MaterialsBuffer;
				dsWrite.m_Buffers[0].m_DescriptorIndex = 0;
				dsWrite.m_Buffers[0].m_Offset = 0;
				dsWrite.m_Buffers[0].m_Size = batchData.m_MaterialDatas->size();

				if (batchData.m_TextureBitsBuffer)
				{
					dsWrite.m_Buffers[1].m_BufferHandle = batchData.m_TextureBitsBuffer;
					dsWrite.m_Buffers[1].m_DescriptorIndex = 1;
					dsWrite.m_Buffers[1].m_Offset = 0;
					dsWrite.m_Buffers[1].m_Size = batchData.m_TextureBits->size()*sizeof(uint32_t);
				}

				pDevice->UpdateDescriptorSet(batchData.m_MaterialSet, dsWrite);
			}

			if (!batchData.m_Pipeline || pPipelineData->IsDirty() || pPipelineData->SettingsDirty())
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
		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::GenerateClusterSSBO" };
		const glm::uvec2 resolution = camera.GetResolution();

		ClusterConstants constants;
		constants.CameraIndex = cameraIndex;
		constants.LightCount = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		constants.GridSize = glm::vec4(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES, 0.0f);

		CommandBufferHandle commandBuffer = pDevice->Begin();
		pDevice->BeginPipeline(commandBuffer, m_ClusterGeneratorPipeline);
		pDevice->BindDescriptorSets(commandBuffer, m_ClusterGeneratorPipeline, { m_GlobalClusterSet, clusterSet });
		pDevice->PushConstants(commandBuffer, m_ClusterGeneratorPipeline, 0, sizeof(ClusterConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
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
				{ AttributeType::Float3, AttributeType::Float4 });
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

	void GloryRendererModule::PrepareSkybox(GraphicsDevice* pDevice)
	{
		GScene* pActiveScene = m_pEngine->GetSceneManager()->GetActiveScene();
		if (!pActiveScene) return;
		const UUID skyboxID = pActiveScene->Settings().m_LightingSettings.m_Skybox;
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(skyboxID);
		if (!pResource)
		{
			m_SkyboxCubemap = 0;
			return;
		}
		CubemapData* pCubemap = static_cast<CubemapData*>(pResource);
		const bool cubemapDirty = pCubemap->IsDirty();
		TextureHandle cubemap = pDevice->AcquireCachedTexture(pCubemap);
		if (!cubemap) return;
		if (!m_GlobalSkyboxSamplerSet)
		{
			m_GlobalSkyboxSamplerSet = CreateSamplerDescriptorSet(pDevice, 1, { cubemap }, m_GlobalSkyboxSamplerSetLayout);
			m_SkyboxCubemap = cubemap;
			return;
		}
		if (!cubemapDirty && cubemap == m_SkyboxCubemap) return;

		DescriptorSetUpdateInfo dsUpdateInfo;
		dsUpdateInfo.m_Samplers = { { cubemap, 0 } };
		pDevice->UpdateDescriptorSet(m_GlobalSkyboxSamplerSet, dsUpdateInfo);
		m_SkyboxCubemap = cubemap;
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
		pDevice->PushConstants(commandBuffer, m_ClusterCullLightPipeline, 0, sizeof(ClusterConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
		pDevice->Dispatch(commandBuffer, 1, 1, 6);
		pDevice->EndPipeline(commandBuffer);
	}

	void GloryRendererModule::SkyboxPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex)
	{
		GScene* pActiveScene = m_pEngine->GetSceneManager()->GetActiveScene();
		if (!pActiveScene) return;
		const UUID skyboxID = pActiveScene->Settings().m_LightingSettings.m_Skybox;
		if (!skyboxID) return;
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(skyboxID);
		if (!pResource) return;

		CameraRef camera = m_ActiveCameras[cameraIndex];
		const glm::uvec2& resolution = camera.GetResolution();

		ProfileSample s{ &m_pEngine->Profiler(), "GloryRendererModule::SkyboxPass" };
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		pDevice->BeginPipeline(commandBuffer, m_SkyboxPipeline);
		pDevice->BindDescriptorSets(commandBuffer, m_SkyboxPipeline, { m_GlobalSkyboxRenderSet, m_GlobalSkyboxSamplerSet });
		pDevice->PushConstants(commandBuffer, m_SkyboxPipeline, 0, sizeof(uint32_t), &cameraIndex, STF_Vertex);
		pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
		pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
		pDevice->DrawUnitCube(commandBuffer);
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
			const auto& lightID = m_FrameData.ActiveLightIDs[i];
			if (!lightData.shadowsEnabled) continue;
			glm::vec4 chunkRect = shadowAtlas.GetChunkPositionAndSize(lightID);
			FixViewport(chunkRect, shadowAtlas.Resolution(), pDevice);
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

		RenderConstants constants;
		constants.m_CameraIndex = static_cast<uint32_t>(lightIndex);
		constants.m_LightCount = m_FrameData.ActiveLights.count();

		pDevice->BeginPipeline(commandBuffer, m_ShadowRenderPipeline);

		size_t batchIndex = 0;
		for (const PipelineBatch& pipelineRenderData : m_DynamicPipelineRenderDatas)
		{
			if (batchIndex >= m_DynamicBatchData.size()) break;
			const PipelineBatchData& batchData = m_DynamicBatchData.at(batchIndex);
			++batchIndex;

			if (viewport.z > 0.0f && viewport.w > 0.0f)
			{
				pDevice->SetViewport(commandBuffer, viewport.x, viewport.y, viewport.z, viewport.w);
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

					pDevice->PushConstants(commandBuffer, batchData.m_Pipeline, 0, sizeof(RenderConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
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
		cameraData.m_VisualizedAttachment = CameraAttachment(DefaultAttachmenmtIndex());
		cameraData.m_RenderPasses.resize(m_ImageCount, 0ull);
		cameraData.m_LateRenderPasses.resize(m_ImageCount, 0ull);
		cameraData.m_SSAORenderPasses.resize(m_ImageCount, 0ull);
		cameraData.m_LightIndexSSBOs.resize(m_ImageCount, 0ull);
		cameraData.m_LightGridSSBOs.resize(m_ImageCount, 0ull);
		cameraData.m_PickResultsSSBOs.resize(m_ImageCount, 0ull);
		cameraData.m_LightSets.resize(m_ImageCount, 0ull);
		cameraData.m_SSAOSamplersSets.resize(m_ImageCount, 0ull);
		cameraData.m_SSAOPostSamplersSets.resize(m_ImageCount, 0ull);
		cameraData.m_ColorSamplerSets.resize(m_ImageCount, 0ull);
		cameraData.m_PickingResultSets.resize(m_ImageCount, 0ull);
		cameraData.m_PickingSamplersSets.resize(m_ImageCount, 0ull);
		cameraData.m_PostProcessPasses.resize(m_ImageCount);

		cameraData.m_ObjectIDSamplerSets.resize(m_ImageCount, 0ull);
		cameraData.m_NormalSamplerSets.resize(m_ImageCount, 0ull);
		cameraData.m_AOSamplerSets.resize(m_ImageCount, 0ull);
		cameraData.m_DepthSamplerSets.resize(m_ImageCount, 0ull);
		cameraData.m_LightGridSets.resize(m_ImageCount, 0ull);

		for (size_t i = 0; i < m_ImageCount; ++i)
		{
			RenderPassHandle& renderPass = cameraData.m_RenderPasses[i];
			RenderPassHandle& lateRenderPass = cameraData.m_LateRenderPasses[i];
			RenderPassHandle& ssaoRenderPass = cameraData.m_SSAORenderPasses[i];
			PostProcessPass& postProcessPass = cameraData.m_PostProcessPasses[i];
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
			if (!postProcessPass.m_BackBufferPass)
			{
				RenderPassInfo renderPassInfo;
				renderPassInfo.RenderTextureInfo.Width = resolution.x;
				renderPassInfo.RenderTextureInfo.Height = resolution.y;
				renderPassInfo.RenderTextureInfo.HasDepth = false;
				renderPassInfo.RenderTextureInfo.HasStencil = false;
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Unorm, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPassInfo.m_Position = RenderPassPosition::RP_Final;
				renderPassInfo.m_LoadOp = RenderPassLoadOp::OP_Clear;
				postProcessPass.m_BackBufferPass = pDevice->CreateRenderPass(std::move(renderPassInfo));

				RenderTextureHandle backBufferTexture = pDevice->GetRenderPassRenderTexture(postProcessPass.m_BackBufferPass);
				DescriptorSetInfo dsInfo;
				dsInfo.m_Layout = m_DisplayCopySamplerSetLayout;
				dsInfo.m_Samplers.resize(1);
				dsInfo.m_Samplers[0].m_TextureHandle = pDevice->GetRenderTextureAttachment(backBufferTexture, 0);
				postProcessPass.m_BackDescriptor = pDevice->CreateDescriptorSet(std::move(dsInfo));
			}
			if (!postProcessPass.m_FrontBufferPass)
			{
				RenderPassInfo renderPassInfo;
				renderPassInfo.RenderTextureInfo.Width = resolution.x;
				renderPassInfo.RenderTextureInfo.Height = resolution.y;
				renderPassInfo.RenderTextureInfo.HasDepth = false;
				renderPassInfo.RenderTextureInfo.HasStencil = false;
				renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Unorm, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
				renderPassInfo.m_Position = RenderPassPosition::RP_Final;
				renderPassInfo.m_LoadOp = RenderPassLoadOp::OP_Clear;
				postProcessPass.m_FrontBufferPass = pDevice->CreateRenderPass(std::move(renderPassInfo));

				RenderTextureHandle frontBufferTexture = pDevice->GetRenderPassRenderTexture(postProcessPass.m_FrontBufferPass);
				DescriptorSetInfo dsInfo;
				dsInfo.m_Layout = m_DisplayCopySamplerSetLayout;
				dsInfo.m_Samplers.resize(1);
				dsInfo.m_Samplers[0].m_TextureHandle = pDevice->GetRenderTextureAttachment(frontBufferTexture, 0);
				postProcessPass.m_FrontDescriptor = pDevice->CreateDescriptorSet(std::move(dsInfo));
			}

			DescriptorSetHandle& lightSet = cameraData.m_LightSets[i];
			DescriptorSetHandle& ssaoSamplersSet = cameraData.m_SSAOSamplersSets[i];
			DescriptorSetHandle& ssaoPostSamplersSet = cameraData.m_SSAOPostSamplersSets[i];
			DescriptorSetHandle& colorSamplerSet = cameraData.m_ColorSamplerSets[i];
			DescriptorSetHandle& pickingResultsSet = cameraData.m_PickingResultSets[i];
			DescriptorSetHandle& pickingSamplersSet = cameraData.m_PickingSamplersSets[i];
			BufferHandle& lightIndexSSBO = cameraData.m_LightIndexSSBOs[i];
			BufferHandle& lightGridSSBO = cameraData.m_LightGridSSBOs[i];
			BufferHandle& pickResultsUBO = cameraData.m_PickResultsSSBOs[i];
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			RenderTextureHandle ssaoRenderTexture = pDevice->GetRenderPassRenderTexture(ssaoRenderPass);
			TextureHandle objectID = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 1);
			TextureHandle normals = pDevice->GetRenderTextureAttachment(renderTexture, 2);
			TextureHandle depth = pDevice->GetRenderTextureAttachment(renderTexture, 3);
			TextureHandle ao = pDevice->GetRenderTextureAttachment(ssaoRenderTexture, 0);

			DescriptorSetHandle& objectIDSamplerSet = cameraData.m_ObjectIDSamplerSets[i];
			DescriptorSetHandle& normalSamplerSet = cameraData.m_NormalSamplerSets[i];
			DescriptorSetHandle& aoSamplerSet = cameraData.m_AOSamplerSets[i];
			DescriptorSetHandle& depthSamplerSet = cameraData.m_DepthSamplerSets[i];
			DescriptorSetHandle& lightGridSet = cameraData.m_LightGridSets[i];

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
			
			if (!ssaoPostSamplersSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_SSAOPostSamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = ao;
				ssaoPostSamplersSet = pDevice->CreateDescriptorSet(std::move(setInfo));
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
			
			if (!objectIDSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = objectID;
				objectIDSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!normalSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = normals;
				normalSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}
			
			if (!aoSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = ao;
				aoSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}
			
			if (!depthSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = depth;
				depthSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!lightGridSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = m_LightGridSetLayout;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = lightGridSSBO;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(LightGrid) * NUM_CLUSTERS;
				lightGridSet = pDevice->CreateDescriptorSet(std::move(setInfo));
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

			glm::mat4 projection = camera.GetProjection();
			glm::mat4 projectionInverse = camera.GetProjectionInverse();
			FixProjection(projection, pDevice);
			FixProjection(projectionInverse, pDevice);

			const PerCameraData cameraData{ camera.GetView(), projection, camera.GetViewInverse(), projectionInverse,
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

	void GloryRendererModule::CheckCachedPipelines(GraphicsDevice* pDevice)
	{
		const ModuleSettings& settings = Settings();
		const UUID clusterGeneratorPipeline = settings.Value<uint64_t>("Cluster Generator");
		const UUID clusterCullLightPipeline = settings.Value<uint64_t>("Cluster Cull Light");
		const UUID pickingPipeline = settings.Value<uint64_t>("Picking");
		const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");
		const UUID ssaoPostPassPipeline = settings.Value<uint64_t>("SSAO Postpass");
		const UUID visualizeSSAOPipeline = settings.Value<uint64_t>("SSAO Visualizer");
		const UUID visualizeObjectIDPipeline = settings.Value<uint64_t>("ObjectID Visualizer");
		const UUID visualizeDepthPipeline = settings.Value<uint64_t>("Depth Visualizer");
		const UUID visualizeLightComplexityPipeline = settings.Value<uint64_t>("Light Complexity Visualizer");

		static DescriptorSetLayoutHandle doubleFloatPushConstantsLayout = NULL;

		if (!doubleFloatPushConstantsLayout)
		{
			DescriptorSetLayoutInfo ssaoConstantsInfo;
			ssaoConstantsInfo.m_PushConstantRange.m_Offset = 0;
			ssaoConstantsInfo.m_PushConstantRange.m_ShaderStages = STF_Fragment;
			ssaoConstantsInfo.m_PushConstantRange.m_Size = sizeof(float) * 2;
			doubleFloatPushConstantsLayout = pDevice->CreateDescriptorSetLayout(std::move(ssaoConstantsInfo));
		}

		static DescriptorSetLayoutHandle lightComplexityPushConstantsLayout = NULL;

		if (!lightComplexityPushConstantsLayout)
		{
			DescriptorSetLayoutInfo ssaoConstantsInfo;
			ssaoConstantsInfo.m_PushConstantRange.m_Offset = 0;
			ssaoConstantsInfo.m_PushConstantRange.m_ShaderStages = STF_Fragment;
			ssaoConstantsInfo.m_PushConstantRange.m_Size = sizeof(LightComplexityConstants);
			lightComplexityPushConstantsLayout = pDevice->CreateDescriptorSetLayout(std::move(ssaoConstantsInfo));
		}

		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		
		/* Compute */
		PipelineData* pPipeline = pipelines.GetPipelineData(clusterGeneratorPipeline);
		m_ClusterGeneratorPipeline = pDevice->AcquireCachedComputePipeline(pPipeline, { m_GlobalClusterSetLayout, m_CameraClusterSetLayout });
		pPipeline = pipelines.GetPipelineData(clusterCullLightPipeline);
		m_ClusterCullLightPipeline = pDevice->AcquireCachedComputePipeline(pPipeline,
			{ m_GlobalClusterSetLayout, m_CameraClusterSetLayout, m_GlobalLightSetLayout, m_CameraLightSetLayout, m_LightDistancesSetLayout });
		pPipeline = pipelines.GetPipelineData(pickingPipeline);
		m_PickingPipeline = pDevice->AcquireCachedComputePipeline(pPipeline,
			{ m_GlobalPickingSetLayout, m_PickingResultSetLayout, m_PickingSamplerSetLayout });

		/* Graphics */
		pPipeline = pipelines.GetPipelineData(displayPipeline);
		m_DisplayCopyPipeline = pDevice->AcquireCachedPipeline(m_FinalFrameColorPasses[0], pPipeline, { m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(visualizeSSAOPipeline);
		m_VisualizeSSAOPipeline = pDevice->AcquireCachedPipeline(m_FinalFrameColorPasses[0], pPipeline, { m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(visualizeObjectIDPipeline);
		m_VisualizeObjectIDPipeline = pDevice->AcquireCachedPipeline(m_FinalFrameColorPasses[0], pPipeline, { m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(visualizeDepthPipeline);
		m_VisualizeDepthPipeline = pDevice->AcquireCachedPipeline(m_FinalFrameColorPasses[0], pPipeline,
			{ doubleFloatPushConstantsLayout, m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(visualizeLightComplexityPipeline);
		m_VisualizeLightComplexityPipeline = pDevice->AcquireCachedPipeline(m_FinalFrameColorPasses[0], pPipeline,
			{ lightComplexityPushConstantsLayout, m_DisplayCopySamplerSetLayout, m_LightGridSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(ssaoPostPassPipeline);
		m_SSAOPostPassPipeline = pDevice->AcquireCachedPipeline(m_FinalFrameColorPasses[0], pPipeline,
			{ doubleFloatPushConstantsLayout, m_DisplayCopySamplerSetLayout, m_SSAOPostSamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
		pPipeline = pipelines.GetPipelineData(shadowsPipeline);
		m_ShadowRenderPipeline = pDevice->AcquireCachedPipeline(m_ShadowsPasses[0], pPipeline,
			{ m_GlobalShadowRenderSetLayout, m_ObjectDataSetLayout }, sizeof(DefaultVertex3D),
			{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
			AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });
		const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");
		pPipeline = pipelines.GetPipelineData(shadowsTransparentPipeline);
		//m_TransparentShadowRenderPipeline = pDevice->AcquireCachedPipeline(m_ShadowsPasses[0], pPipeline, {}, sizeof(DefaultVertex3D),
		//	{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
		//	AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });
		const UUID ssaoPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
		pPipeline = pipelines.GetPipelineData(ssaoPipeline);
		m_SSAOPipeline = pDevice->AcquireCachedPipeline(m_DummySSAORenderPass, pPipeline,
			{ m_GlobalClusterSetLayout, m_GlobalSampleDomeSetLayout, m_SSAOSamplersSetLayout, m_NoiseSamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID skyboxPipeline = settings.Value<uint64_t>("Skybox Pipeline");
		pPipeline = pipelines.GetPipelineData(skyboxPipeline);
		m_SkyboxPipeline = pDevice->AcquireCachedPipeline(m_DummyRenderPass, pPipeline,
			{ m_GlobalSkyboxRenderSetLayout, m_GlobalSkyboxSamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID lineRenderPipeline = settings.Value<uint64_t>("Lines Pipeline");
		pPipeline = pipelines.GetPipelineData(lineRenderPipeline);
		m_LineRenderPipeline = pDevice->AcquireCachedPipeline(m_DummyRenderPass, pPipeline,
			{ m_GlobalLineRenderSetLayout }, sizeof(LineVertex),
			{ AttributeType::Float3, AttributeType::Float4 });
	}
}
