#include "GloryRenderer.h"
#include "GloryRendererModule.h"

#include <Engine.h>
#include <Console.h>
#include <GScene.h>
#include <SceneManager.h>
#include <GraphicsDevice.h>
#include <ModuleSettings.h>
#include <DescriptorHelpers.h>
#include <EngineProfiler.h>
#include <GPUTextureAtlas.h>
#include <RenderHelpers.h>

#include <AssetManager.h>
#include <PipelineManager.h>
#include <MaterialManager.h>

#include <PipelineData.h>
#include <MeshData.h>
#include <MaterialData.h>
#include <CubemapData.h>

#include <random>

namespace Glory
{
	static const size_t GridSizeX = 16;
	static const size_t GridSizeY = 9;
	static const size_t NUM_DEPTH_SLICES = 24;
	static const size_t NUM_CLUSTERS = GridSizeX*GridSizeY*NUM_DEPTH_SLICES;
	static const size_t MAX_LIGHTS_PER_TILE = 50;
	static const size_t MAX_KERNEL_SIZE = 1024;

	GloryRenderer::GloryRenderer(): m_pModule(nullptr), Renderer(nullptr)
	{
	}

	GloryRenderer::GloryRenderer(GloryRendererModule* pModule) : m_pModule(pModule), Renderer(pModule)
	{
	}

	GloryRenderer::GloryRenderer(const GloryRenderer& other) : m_pModule(other.m_pModule), Renderer(other.m_pModule)
	{
	}

	GloryRenderer::~GloryRenderer()
	{
	}

	void GloryRenderer::OnCameraResize(CameraRef camera)
	{
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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

	void GloryRenderer::OnCameraPerspectiveChanged(CameraRef camera)
	{
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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

	MaterialData* GloryRenderer::GetInternalMaterial(std::string_view name) const
	{
		return nullptr;
	}

	void GloryRenderer::PresentFrame()
	{
		if (!m_Enabled) return;

		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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

	uint32_t GloryRenderer::GetNumFramesInFlight() const
	{
		return m_ImageCount;
	}

	uint32_t GloryRenderer::GetCurrentFrameInFlight() const
	{
		return m_CurrentFrameIndex;
	}

	void GloryRenderer::Initialize()
	{
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		if (!pDevice)
		{
			m_pModule->GetEngine()->GetDebug().LogError("Renderer: No graphics device active");
			return;
		}
	
		/* Global data buffers */
		m_CameraDatasBuffer = pDevice->CreateBuffer(sizeof(PerCameraData) * MAX_CAMERAS, BufferType::BT_Storage, BF_Write);
		m_LightCameraDatasBuffer = pDevice->CreateBuffer(sizeof(PerCameraData) * MAX_LIGHTS, BufferType::BT_Storage, BF_Write);
		m_LightsSSBO = pDevice->CreateBuffer(sizeof(LightData) * MAX_LIGHTS, BufferType::BT_Storage, BF_Write);
		m_LightSpaceTransformsSSBO = pDevice->CreateBuffer(sizeof(glm::mat4) * MAX_LIGHTS, BufferType::BT_Storage, BF_Write);
	
		GenerateDomeSamplePointsSSBO(pDevice, 64);
		GenerateNoiseTexture(pDevice);
	
		/* Global set */
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Vertex | STF_Fragment) }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData) * MAX_CAMERAS } },
			RendererDSLayouts::m_GlobalRenderSetLayout, m_GlobalRenderSet, ShaderTypeFlag(STF_Vertex | STF_Fragment), 0, sizeof(RenderConstants));
	
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Vertex | STF_Fragment) }, { m_LightCameraDatasBuffer }, { { 0, sizeof(PerCameraData) * MAX_LIGHTS } },
			RendererDSLayouts::m_GlobalShadowRenderSetLayout, m_GlobalShadowRenderSet, ShaderTypeFlag(STF_Vertex | STF_Fragment), 0, sizeof(RenderConstants));
	
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Compute }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData) * MAX_CAMERAS } },
			RendererDSLayouts::m_GlobalPickingSetLayout, m_GlobalPickingSet, STF_Compute, 0, sizeof(PickingConstants));
	
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Vertex }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData) * MAX_CAMERAS } },
			RendererDSLayouts::m_GlobalLineRenderSetLayout, m_GlobalLineRenderSet, STF_Vertex, 0, sizeof(uint32_t));
	
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { STF_Vertex }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData) * MAX_CAMERAS } },
			RendererDSLayouts::m_GlobalSkyboxRenderSetLayout, m_GlobalSkyboxRenderSet, STF_Vertex, 0, sizeof(uint32_t));
	
		assert(RendererDSLayouts::m_GlobalRenderSetLayout == RendererDSLayouts::m_GlobalShadowRenderSetLayout);
	
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::CameraDatas },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) }, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData) * MAX_CAMERAS } },
			RendererDSLayouts::m_GlobalClusterSetLayout, m_GlobalClusterSet, ShaderTypeFlag(STF_Compute | STF_Fragment), 0, sizeof(ClusterConstants));
	
		RendererDSLayouts::m_CameraClusterSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::Clusters },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) });
	
		CreateBufferDescriptorLayoutAndSet(pDevice, 2, { BufferBindingIndices::LightDatas, BufferBindingIndices::LightSpaceTransforms },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) }, { m_LightsSSBO, m_LightSpaceTransformsSSBO }, { { 0, sizeof(LightData) * MAX_LIGHTS }, { 0, sizeof(glm::mat4) * MAX_LIGHTS } },
			RendererDSLayouts::m_GlobalLightSetLayout, m_GlobalLightSet);
	
		RendererDSLayouts::m_CameraLightSetLayout = CreateBufferDescriptorLayout(pDevice, 3, { BufferBindingIndices::LightIndices, BufferBindingIndices::LightGrid, BufferBindingIndices::LightDistances },
			{ BufferType::BT_Storage }, { ShaderTypeFlag(STF_Compute | STF_Fragment) });
	
		m_SSAOCameraSet = CreateBufferDescriptorSet(pDevice, 1, { m_CameraDatasBuffer }, { { 0, sizeof(PerCameraData) * MAX_CAMERAS } },
			RendererDSLayouts::m_GlobalClusterSetLayout, 0, sizeof(SSAOConstants));
	
		CreateBufferDescriptorLayoutAndSet(pDevice, 1, { BufferBindingIndices::SampleDome },
			{ BufferType::BT_Uniform }, { STF_Fragment }, { m_SamplePointsDomeSSBO },
			{ {0, sizeof(glm::vec3) * m_SSAOKernelSize} }, RendererDSLayouts::m_GlobalSampleDomeSetLayout, m_GlobalSampleDomeSet);
	
		RendererDSLayouts::m_SSAOSamplersSetLayout = CreateSamplerDescriptorLayout(pDevice, 2, { 0, 1 }, { STF_Fragment }, { "Normal", "Depth" });
		RendererDSLayouts::m_NoiseSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 2 }, { STF_Fragment }, { "Noise" });
		RendererDSLayouts::m_CameraSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 6, { 0, 1, 2, 3, 4, 5 }, { STF_Fragment }, { "Color", "Normal", "Depth" });
		m_NoiseSamplerSet = CreateSamplerDescriptorSet(pDevice, 1, { m_SampleNoiseTexture }, RendererDSLayouts::m_NoiseSamplerSetLayout);
		RendererDSLayouts::m_DisplayCopySamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "Color" });
		RendererDSLayouts::m_SSAOPostSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 1 }, { STF_Fragment }, { "AO" });
		RendererDSLayouts::m_ShadowAtlasSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "ShadowAtlas" });
		RendererDSLayouts::m_ObjectDataSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::WorldTransforms }, { BT_Storage }, { STF_Vertex });
		RendererDSLayouts::m_LightDistancesSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::LightDistances }, { BT_Storage }, { STF_Compute });
	
		RendererDSLayouts::m_PickingResultSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { BufferBindingIndices::PickingResults }, { BT_Storage }, { STF_Compute });
		RendererDSLayouts::m_PickingSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 3, { 0, 1, 2 }, { STF_Compute }, { "ObjectID", "Normal", "Depth" });
		RendererDSLayouts::m_GlobalSkyboxSamplerSetLayout = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "Skybox" });
		RendererDSLayouts::m_LightGridSetLayout = CreateBufferDescriptorLayout(pDevice, 1, { 1 }, { BT_Storage }, { STF_Fragment });
	
		ResetLightDistances = new uint32_t[MAX_LIGHTS];
		for (size_t i = 0; i < MAX_LIGHTS; ++i)
			ResetLightDistances[i] = NUM_DEPTH_SLICES;
	
		assert(m_ImageCount > 0);
		m_ShadowsPasses.resize(m_ImageCount);
		m_ShadowAtlasses.resize(m_ImageCount);
		m_ShadowAtlasSamplerSets.resize(m_ImageCount);
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();
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
			m_ShadowAtlasSamplerSets[i] = CreateSamplerDescriptorSet(pDevice, 1, { texture }, RendererDSLayouts::m_ShadowAtlasSamplerSetLayout);
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
			dsInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
			dsInfo.m_Samplers.resize(1);
			dsInfo.m_Samplers[0].m_TextureHandle = color;
			m_FinalFrameColorSets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
		}
	
		m_LightDistancesSSBOs.resize(m_ImageCount, 0ull);
		m_LightDistancesSets.resize(m_ImageCount, 0ull);
		for (size_t i = 0; i < m_LightDistancesSSBOs.size(); ++i)
		{
			if (!m_LightDistancesSSBOs[i])
				m_LightDistancesSSBOs[i] = pDevice->CreateBuffer(sizeof(uint32_t) * MAX_LIGHTS, BufferType::BT_Storage, BF_ReadAndWrite);
	
			if (!m_LightDistancesSets[i])
			{
				DescriptorSetInfo dsInfo;
				dsInfo.m_Layout = RendererDSLayouts::m_LightDistancesSetLayout;
				dsInfo.m_Buffers.resize(1);
				dsInfo.m_Buffers[0].m_BufferHandle = m_LightDistancesSSBOs[i];
				dsInfo.m_Buffers[0].m_Offset = 0;
				dsInfo.m_Buffers[0].m_Size = sizeof(uint32_t) * MAX_LIGHTS;
				m_LightDistancesSets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
			}
		}
		m_ClosestLightDepthSlices.resize(MAX_LIGHTS, NUM_DEPTH_SLICES);
		GenerateShadowLODDivisions(m_MaxShadowLODs);
		GenerateShadowMapLODResolutions();
	
		m_LineBuffers.resize(m_ImageCount, 0ull);
		m_LineMeshes.resize(m_ImageCount, 0ull);
	
		PostProcess displayCopyPP;
		displayCopyPP.m_Name = "Initial Display Copy";
		displayCopyPP.m_Priority = INT32_MAX;
		displayCopyPP.m_Callback = [this](GraphicsDevice* pDevice, CameraRef camera, size_t,
			CommandBufferHandle commandBuffer, size_t, RenderPassHandle renderPass, DescriptorSetHandle) {
				const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
	
				const glm::uvec2& resolution = camera.GetResolution();
				pDevice->BeginRenderPass(commandBuffer, renderPass);
				pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_DisplayCopyPipeline);
				pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
				pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
				pDevice->BindDescriptorSets(commandBuffer, RendererPipelines::m_DisplayCopyPipeline, { uniqueCameraData.m_ColorSamplerSets[m_CurrentFrameIndex] });
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
				pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_SSAOPostPassPipeline);
				pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
				pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
				pDevice->BindDescriptorSets(commandBuffer, RendererPipelines::m_SSAOPostPassPipeline, { colorSet, ssaoSamplersSet });
				pDevice->PushConstants(commandBuffer, RendererPipelines::m_SSAOPostPassPipeline, 0, sizeof(aoSettings), aoSettings, STF_Fragment);
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
					previewPipeline = RendererPipelines::m_VisualizeObjectIDPipeline;
					previewSamplerSet = uniqueCameraData.m_ObjectIDSamplerSets[m_CurrentFrameIndex];
					break;
				case CameraAttachment::Color:
					previewPipeline = RendererPipelines::m_DisplayCopyPipeline;
					previewSamplerSet = uniqueCameraData.m_ColorSamplerSets[m_CurrentFrameIndex];
					break;
				case CameraAttachment::Normal:
					previewPipeline = RendererPipelines::m_DisplayCopyPipeline;
					previewSamplerSet = uniqueCameraData.m_NormalSamplerSets[m_CurrentFrameIndex];
					break;
				case CameraAttachment::AO:
					previewPipeline = RendererPipelines::m_VisualizeSSAOPipeline;
					previewSamplerSet = uniqueCameraData.m_AOSamplerSets[m_CurrentFrameIndex];
					break;
				case CameraAttachment::Depth:
					previewPipeline = RendererPipelines::m_VisualizeDepthPipeline;
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
				pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_DisplayCopyPipeline);
				pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
				pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
				pDevice->BindDescriptorSets(commandBuffer, RendererPipelines::m_DisplayCopyPipeline, { colorSet });
				pDevice->DrawQuad(commandBuffer);
				pDevice->EndPipeline(commandBuffer);
	
				if (lightComplexity)
				{
					pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_VisualizeLightComplexityPipeline);
					pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
					pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
	
					LightComplexityConstants constants;
					constants.zNear = camera.GetNear();
					constants.zFar = camera.GetFar();
					constants.Resolution = camera.GetResolution();
					constants.GridSize = glm::uvec4(GridSizeX, GridSizeY, NUM_DEPTH_SLICES, 0.0f);
	
					pDevice->PushConstants(commandBuffer, RendererPipelines::m_VisualizeLightComplexityPipeline, 0, sizeof(LightComplexityConstants), &constants, STF_Fragment);
					pDevice->BindDescriptorSets(commandBuffer, RendererPipelines::m_VisualizeLightComplexityPipeline,
						{ uniqueCameraData.m_DepthSamplerSets[m_CurrentFrameIndex], uniqueCameraData.m_LightGridSets[m_CurrentFrameIndex] });
					pDevice->DrawQuad(commandBuffer);
					pDevice->EndPipeline(commandBuffer);
				}
	
				if (shadowAtlas)
				{
					const uint32_t size = resolution.y / 2;
	
					pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_VisualizeDepthPipeline);
					pDevice->SetViewport(commandBuffer, 0.0f, float(resolution.y - size), float(size), float(size));
					pDevice->SetScissor(commandBuffer, 0, resolution.y - size, size, size);
					float constants[2] = {
						camera.GetNear(),
						camera.GetFar(),
					};
					pDevice->PushConstants(commandBuffer, RendererPipelines::m_VisualizeDepthPipeline, 0, sizeof(constants), constants, STF_Fragment);
					pDevice->BindDescriptorSets(commandBuffer, RendererPipelines::m_VisualizeDepthPipeline, { m_ShadowAtlasSamplerSets[m_CurrentFrameIndex] });
					pDevice->DrawQuad(commandBuffer);
					pDevice->EndPipeline(commandBuffer);
				}
	
				pDevice->EndRenderPass(commandBuffer);
				return true;
			};
	
		AddPostProcess(std::move(debugOverlayPP));
	}

	void GloryRenderer::InitializeAsMainRenderer()
	{
		const ModuleSettings& settings = m_pModule->Settings();
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

		m_pModule->GetEngine()->GetConsole().RegisterCVar({ std::string{ RendererCVARs::ScreenSpaceAOCVarName }, "Enables/disables screen space ambient occlusion.",
			float(m_GlobalSSAOSetting.m_Enabled), CVar::Flags::Save });
		m_pModule->GetEngine()->GetConsole().RegisterCVar({ std::string{ RendererCVARs::MinShadowResolutionVarName }, "Sets the minimum resolution for shadow maps.",
			float(m_MinShadowResolution), CVar::Flags::Save });
		m_pModule->GetEngine()->GetConsole().RegisterCVar({ std::string{ RendererCVARs::MaxShadowResolutionVarName }, "Sets the maximum resolution for shadow maps.",
			float(m_MaxShadowResolution), CVar::Flags::Save });
		m_pModule->GetEngine()->GetConsole().RegisterCVar({ std::string{ RendererCVARs::ShadowAtlasResolution }, "Sets the resolution for the shadow atlas.",
			float(m_ShadowAtlasResolution), CVar::Flags::Save });
		m_pModule->GetEngine()->GetConsole().RegisterCVar({ std::string{ RendererCVARs::MaxShadowLODs }, "Sets the number of shadow map LODs.",
			float(m_MaxShadowLODs), CVar::Flags::Save });

		m_pModule->GetEngine()->GetConsole().RegisterCVar({ std::string{ RendererCVARs::CameraOutputAttachment }, "Sets which attachment on the camera should be outputed.",
			float(DefaultAttachmenmtIndex()), CVar::Flags::None });
		m_pModule->GetEngine()->GetConsole().RegisterCVar({ std::string{ RendererCVARs::VisualizeShadowAtlas }, "Enables/disables shadow atlas debug overlay.",
			float(DefaultAttachmenmtIndex()), CVar::Flags::None });
		m_pModule->GetEngine()->GetConsole().RegisterCVar({ std::string{ RendererCVARs::VisualizeLightComplexity }, "Enables/disables light complexity debug overlay.",
			float(DefaultAttachmenmtIndex()), CVar::Flags::None });

		m_pModule->GetEngine()->GetConsole().RegisterCVarChangeHandler(std::string{ RendererCVARs::ScreenSpaceAOCVarName }, [this](const CVar* cvar) {
			m_GlobalSSAOSetting.m_Enabled = cvar->m_Value == 1.0f;
			m_GlobalSSAOSetting.m_Dirty = true;
		});

		m_pModule->GetEngine()->GetConsole().RegisterCVarChangeHandler(std::string{ RendererCVARs::MinShadowResolutionVarName }, [this](const CVar* cvar) {
			ResizeShadowMapLODResolutions(uint32_t(cvar->m_Value), m_MaxShadowResolution);
		});

		m_pModule->GetEngine()->GetConsole().RegisterCVarChangeHandler(std::string{ RendererCVARs::MaxShadowResolutionVarName }, [this](const CVar* cvar) {
			ResizeShadowMapLODResolutions(m_MinShadowResolution, uint32_t(cvar->m_Value));
		});

		m_pModule->GetEngine()->GetConsole().RegisterCVarChangeHandler(std::string{ RendererCVARs::MaxShadowLODs }, [this](const CVar* cvar) {
			m_MaxShadowLODs = cvar->m_Value;
			GenerateShadowLODDivisions(cvar->m_Value);
			ResizeShadowMapLODResolutions(m_MinShadowResolution, m_MaxShadowResolution);
		});

		m_pModule->GetEngine()->GetConsole().RegisterCVarChangeHandler(std::string{ RendererCVARs::CameraOutputAttachment }, [this](const CVar* cvar) {
			for (size_t i = 0; i < m_OutputCameras.size(); ++i)
				VisualizeAttachment(m_OutputCameras[i], size_t(cvar->m_Value));
		});

		m_pModule->GetEngine()->GetConsole().RegisterCVarChangeHandler(std::string{ RendererCVARs::VisualizeShadowAtlas }, [this](const CVar* cvar) {
			SetDebugOverlayEnabled(NULL, DebugOverlayBitIndices::ShadowAtlas, cvar->m_Value != 0.0f);
		});

		m_pModule->GetEngine()->GetConsole().RegisterCVarChangeHandler(std::string{ RendererCVARs::VisualizeLightComplexity }, [this](const CVar* cvar) {
			SetDebugOverlayEnabled(NULL, DebugOverlayBitIndices::LightComplexity, cvar->m_Value != 0.0f);
		});
	}

	//void GloryRenderer::Update()
	//{
	//	ModuleSettings& settings = Settings();
	//	if (!settings.IsDirty()) return;
	//
	//	const UUID linesPipeline = settings.Value<uint64_t>("Lines Pipeline");
	//	const UUID screenPipeline = settings.Value<uint64_t>("Screen Pipeline");
	//	const UUID SSAOPrePassPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
	//	const UUID SSAOBlurPipeline = settings.Value<uint64_t>("SSAO Blur Pipeline");
	//	const UUID textPipeline = settings.Value<uint64_t>("Text Pipeline");
	//	const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");
	//	const UUID skyboxPipeline = settings.Value<uint64_t>("Skybox Pipeline");
	//	const UUID irradiancePipeline = settings.Value<uint64_t>("Irradiance Pipeline");
	//	const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
	//	const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");
	//	const UUID clusterGeneratorPipeline = settings.Value<uint64_t>("Cluster Generator");
	//	const UUID clusterCullLightPipeline = settings.Value<uint64_t>("Cluster Cull Light");
	//	const UUID pickingPipeline = settings.Value<uint64_t>("Picking");
	//	const UUID ssaoPostpassPipeline = settings.Value<uint64_t>("SSAO Postpass");
	//	const UUID ssaoVisualizerPipeline = settings.Value<uint64_t>("SSAO Visualizer");
	//	const UUID objectIDVisualizerPipeline = settings.Value<uint64_t>("ObjectID Visualizer");
	//	const UUID depthVisualizerPipeline = settings.Value<uint64_t>("Depth Visualizer");
	//	const UUID lightComplexityVisualizerPipeline = settings.Value<uint64_t>("Light Complexity Visualizer");
	//
	//	settings.SetDirty(false);
	//}

	void GloryRenderer::Draw()
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw" };
		if (!m_Enabled) return;

		const ModuleSettings& settings = m_pModule->Settings();
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();

		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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
				pDevice->ReadBuffer(lightDistancesSSBO, m_ClosestLightDepthSlices.data(), 0, m_FrameData.ActiveLights.count() * sizeof(uint32_t));

				/* @todo: Reset using a compute shader */
				pDevice->AssignBuffer(lightDistancesSSBO, ResetLightDistances, 0, m_FrameData.ActiveLights.count() * sizeof(uint32_t));

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
					pDevice->ReadBuffer(pickResults, results.data(), sizeof(uint32_t) * 4, numPicks * sizeof(GPUPickResult));
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
			m_pModule->GetEngine()->GetDebug().LogError("Failed to wait for render finished!");
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
				uniqueCameraData.m_ClusterSSBO = pDevice->CreateBuffer(sizeof(VolumeTileAABB) * NUM_CLUSTERS, BufferType::BT_Storage, BF_CopyDst);
				DescriptorSetInfo setInfo;
				setInfo.m_Layout = RendererDSLayouts::m_CameraClusterSetLayout;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = clusterSSBOHandle;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(VolumeTileAABB) * NUM_CLUSTERS;
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
			ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Camera " + std::to_string(i) + " light cluster culling" };
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
			ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Camera " + std::to_string(i) + " forward pass" };
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
				pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_LineRenderPipeline);
				pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_LineRenderPipeline, { m_GlobalLineRenderSet });
				pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_LineRenderPipeline, 0, sizeof(uint32_t), &cameraIndex, STF_Vertex);
				pDevice->DrawMesh(m_FrameCommandBuffers[m_CurrentFrameIndex], m_LineMeshes[m_CurrentFrameIndex]);
				pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			}
			pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}

		/* Picking */
		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Camera " + std::to_string(i) + " picking" };
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
			std::memcpy(constants.m_Picks, uniqueCameraData.m_Picks.data(), count * sizeof(glm::ivec2));

			pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_PickingPipeline);
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_PickingPipeline,
				{ m_GlobalPickingSet, pickingResultSet, pickingSamplersSet });
			pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_PickingPipeline,
				0, sizeof(PickingConstants), &constants, STF_Compute);
			pDevice->Dispatch(m_FrameCommandBuffers[m_CurrentFrameIndex], 1, 1, 1);
			pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}

		for (size_t i = 0; i < m_ActiveCameras.size(); ++i)
		{
			ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Camera " + std::to_string(i) + " late" };
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
				ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Camera " + std::to_string(i) + " SSAO pass" };
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
				pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_SSAOPipeline);
				pDevice->SetViewport(m_FrameCommandBuffers[m_CurrentFrameIndex], 0.0f, 0.0f, float(resolution.x), float(resolution.y));
				pDevice->SetScissor(m_FrameCommandBuffers[m_CurrentFrameIndex], 0, 0, resolution.x, resolution.y);
				pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_SSAOPipeline, { m_SSAOCameraSet, m_GlobalSampleDomeSet, ssaoSamplersSet, m_NoiseSamplerSet });
				pDevice->PushConstants(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_SSAOPipeline, 0, sizeof(SSAOConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
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
			ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Camera " + std::to_string(i) + " Post Processing" };
			CameraRef camera = m_ActiveCameras[i];
			UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			PostProcessPass& postProcessPass = uniqueCameraData.m_PostProcessPasses[m_CurrentFrameIndex];

			for (const PostProcess& pp : m_PostProcesses)
			{
				ProfileSample profile{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Camera Post Process: " + pp.m_Name };
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
		pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_DisplayCopyPipeline);
		for (size_t i = 0; i < m_OutputCameras.size(); ++i)
		{
			ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Render output from camera " + std::to_string(i) };
			CameraRef camera = m_OutputCameras[i];
			const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_DisplayCopyPipeline,
				{ uniqueCameraData.m_PostProcessPasses[m_CurrentFrameIndex].m_FrontDescriptor });
			pDevice->DrawQuad(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}
		pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);

		if (m_Swapchain)
		{
			ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::Draw: Render to swap chain image" };
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[m_CurrentFrameIndex]);
			TextureHandle color = pDevice->GetRenderTextureAttachment(renderTexture, 0);

			ImageBarrier colorBarrier;
			colorBarrier.m_Texture = color;
			colorBarrier.m_SrcAccessMask = AF_ColorAttachmentWrite;
			colorBarrier.m_DstAccessMask = AF_ShaderRead;
			pDevice->PipelineBarrier(m_FrameCommandBuffers[m_CurrentFrameIndex], {}, { colorBarrier },
				PipelineStageFlagBits::PST_ColorAttachmentOutput, PipelineStageFlagBits::PST_FragmentShader);

			pDevice->BeginRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex], m_SwapchainPasses[m_CurrentSemaphoreIndex]);
			pDevice->BeginPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_DisplayCopyPipeline);
			pDevice->BindDescriptorSets(m_FrameCommandBuffers[m_CurrentFrameIndex], RendererPipelines::m_DisplayCopyPipeline,
				{ m_FinalFrameColorSets[m_CurrentFrameIndex] });
			pDevice->DrawQuad(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			pDevice->EndPipeline(m_FrameCommandBuffers[m_CurrentFrameIndex]);
			for (auto& injectedSubpass : m_InjectedSwapchainSubpasses)
				injectedSubpass(pDevice, m_SwapchainPasses[m_CurrentSemaphoreIndex], m_FrameCommandBuffers[m_CurrentFrameIndex]);
			pDevice->EndRenderPass(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		}
		pDevice->End(m_FrameCommandBuffers[m_CurrentFrameIndex]);
		pDevice->Commit(m_FrameCommandBuffers[m_CurrentFrameIndex], waitSemaphores, signalSemaphores);

		if (!m_Swapchain)
		{
			++m_CurrentFrameIndex;
			m_CurrentFrameIndex = m_CurrentFrameIndex % m_ImageCount;
		}
	}

	Renderer* GloryRenderer::CreateSecondaryRenderer(size_t imageCount)
	{
		return m_pModule->CreateSecondaryRenderer(imageCount);
	}

	uint32_t GloryRenderer::NextFrameIndex()
	{
		return m_CurrentFrameIndex;
	}

	bool GloryRenderer::FrameBusy(uint32_t frameIndex)
	{
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		if (!pDevice) return false;

		if (!m_FrameCommandBuffers[frameIndex]) return false;
		const GraphicsDevice::WaitResult result = pDevice->Wait(m_FrameCommandBuffers[frameIndex], 1);
		switch (result)
		{
		case GraphicsDevice::WR_Timeout:
			return true;
		case GraphicsDevice::WR_Success:
			return false;
		}

		m_pModule->GetEngine()->GetDebug().LogError("Failed to wait for render finished!");
		return false;
	}

	void GloryRenderer::Cleanup()
	{
	}

	size_t GloryRenderer::DefaultAttachmenmtIndex() const
	{
		return 5;
	}

	size_t GloryRenderer::CameraAttachmentPreviewCount() const
	{
		return AttachmentNameCount;
	}

	std::string_view GloryRenderer::CameraAttachmentPreviewName(size_t index) const
	{
		return AttachmentNames[index];
	}

	TextureHandle GloryRenderer::CameraAttachmentPreview(CameraRef camera, size_t index) const
	{
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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
			return pDevice->GetRenderTextureAttachment(renderTexture, index - 4);
		}

		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
		return pDevice->GetRenderTextureAttachment(renderTexture, index);
	}

	TextureHandle GloryRenderer::FinalColor() const
	{
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[m_CurrentFrameIndex]);
		return pDevice->GetRenderTextureAttachment(renderTexture, 0);
	}

	TextureHandle GloryRenderer::FinalColor(uint32_t frameIndex) const
	{
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_FinalFrameColorPasses[frameIndex]);
		return pDevice->GetRenderTextureAttachment(renderTexture, 0);
	}

	void GloryRenderer::VisualizeAttachment(CameraRef camera, size_t index)
	{
		if (index >= CameraAttachment::Count) return;
		UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		uniqueCameraData.m_VisualizedAttachment = CameraAttachment(index);
	}

	size_t GloryRenderer::DebugOverlayCount() const
	{
		return DebugOverlayNameCount;
	}

	std::string_view GloryRenderer::DebugOverlayName(size_t index) const
	{
		return DebugOverlayNames[index];
	}

	void GloryRenderer::SetDebugOverlayEnabled(CameraRef camera, size_t index, bool enabled)
	{
		if (camera == NULL)
		{
			m_DebugOverlayBits.Set(index, enabled);
			return;
		}

		UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		uniqueCameraData.m_DebugOverlayBits.Set(index, enabled);
	}

	bool GloryRenderer::DebugOverlayEnabled(CameraRef camera, size_t index) const
	{
		if (camera == NULL)
			return m_DebugOverlayBits.IsSet(index);

		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		return uniqueCameraData.m_DebugOverlayBits.IsSet(index);
	}

	void GloryRenderer::OnWindowResized()
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::OnWindowResized" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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

	void GloryRenderer::OnSwapchainChanged()
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::OnSwapchainChanged" };
		if (!m_Swapchain) return;
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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

		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();
		const ModuleSettings& settings = m_pModule->Settings();
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
				m_ShadowAtlasSamplerSets[i] = CreateSamplerDescriptorSet(pDevice, 1, { texture }, RendererDSLayouts::m_ShadowAtlasSamplerSetLayout);
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
				dsInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
				dsInfo.m_Samplers.resize(1);
				dsInfo.m_Samplers[0].m_TextureHandle = color;
				m_FinalFrameColorSets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
			}

			if (!m_LightDistancesSSBOs[i])
				m_LightDistancesSSBOs[i] = pDevice->CreateBuffer(sizeof(uint32_t) * MAX_LIGHTS, BufferType::BT_Storage, BF_ReadAndWrite);

			if (!m_LightDistancesSets[i])
			{
				DescriptorSetInfo dsInfo;
				dsInfo.m_Layout = RendererDSLayouts::m_LightDistancesSetLayout;
				dsInfo.m_Buffers.resize(1);
				dsInfo.m_Buffers[0].m_BufferHandle = m_LightDistancesSSBOs[i];
				dsInfo.m_Buffers[0].m_Offset = 0;
				dsInfo.m_Buffers[0].m_Size = sizeof(uint32_t) * MAX_LIGHTS;
				m_LightDistancesSets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
			}
		}

		m_CurrentSemaphoreIndex = 0;
		m_CurrentFrameIndex = 0;
	}

	RenderPassHandle GloryRenderer::GetSwapchainPass() const
	{
		return !m_SwapchainPasses.empty() ? m_SwapchainPasses[0] : NULL;
	}

	RenderPassHandle GloryRenderer::GetDummyPostProcessPass() const
	{
		return m_FinalFrameColorPasses[0];
	}

	size_t GloryRenderer::GetGCD(size_t a, size_t b)
	{
		if (b == 0)
			return a;
		return GetGCD(b, a % b);
	}

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void GloryRenderer::RenderBatches(CommandBufferHandle commandBuffer, const std::vector<PipelineBatch>& batches,
		const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex, DescriptorSetHandle globalRenderSet, const glm::vec4& viewport,
		DescriptorSetHandle shadowsSet)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::RenderBatches" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		MaterialManager& materialManager = m_pModule->GetEngine()->GetMaterialManager();
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();
		AssetManager& assets = m_pModule->GetEngine()->GetAssetManager();

		RenderConstants constants;
		constants.m_CameraIndex = static_cast<uint32_t>(cameraIndex);
		constants.m_LightCount = m_FrameData.ActiveLights.count();
		constants.m_GridSize = glm::uvec4(GridSizeX, GridSizeY, NUM_DEPTH_SLICES, 0.0f);
		CameraRef camera = m_ActiveCameras[cameraIndex];
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		const DescriptorSetHandle lightSet = uniqueCameraData.m_LightSets[m_CurrentFrameIndex];
		const LayerMask& cameraMask = camera.GetLayerMask();

		for (auto pipelineID : m_pModule->PipelineOrder())
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
							const glm::vec4 point = world * points[pointIndex];
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

	void GloryRenderer::PrepareDataPass()
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::PrepareDataPass" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		if (!pDevice) return;

		m_pModule->CheckCachedPipelines(pDevice);

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

			const glm::mat4 lightSpace = projection * m_FrameData.LightViews[i];
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
				m_LightCameraDatas.m_DirtyRange.first * sizeof(PerCameraData), dirtySize * sizeof(PerCameraData));
		}

		if (m_LightSpaceTransforms)
		{
			const size_t dirtySize = m_LightSpaceTransforms.DirtySize();
			pDevice->AssignBuffer(m_LightSpaceTransformsSSBO, m_LightSpaceTransforms.DirtyStart(),
				m_LightSpaceTransforms.m_DirtyRange.first * sizeof(glm::mat4), dirtySize * sizeof(glm::mat4));
		}

		/* Prepare shadow resolutions and atlas coords */
		const uint32_t sliceSteps = NUM_DEPTH_SLICES / m_MaxShadowLODs;

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

			const uint32_t shadowLOD = std::min(depthSlice / sliceSteps, uint32_t(m_MaxShadowLODs - 1));
			const glm::uvec2 shadowMapResolution = m_ShadowMapResolutions[shadowLOD];

			const UUID chunkID = shadowAtlas.ReserveChunk(shadowMapResolution.x, shadowMapResolution.y, lightID);
			if (!chunkID)
			{
				lightData.shadowsEnabled = 0;
				m_pModule->GetEngine()->GetDebug().LogError("Failed to reserve chunk in shadow atlas, there is not enough space left.");
				continue;
			}
			lightData.shadowCoords = shadowAtlas.GetChunkCoords(lightID);
			FixShadowCoords(lightData.shadowCoords, pDevice);
		}

		/* Update light data */
		pDevice->AssignBuffer(m_LightsSSBO, m_FrameData.ActiveLights.data(), 0, MAX_LIGHTS * sizeof(LightData));

		PrepareBatches(m_DynamicPipelineRenderDatas, m_DynamicBatchData);
		PrepareBatches(m_DynamicLatePipelineRenderDatas, m_DynamicLateBatchData);
		PrepareLineMesh(pDevice);
		PrepareSkybox(pDevice);
	}

	void GloryRenderer::PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::PrepareBatches" };
		if (m_ActiveCameras.empty()) return;
		CameraRef defaultCamera = m_ActiveCameras[0];
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(defaultCamera.GetUUID());
		const RenderPassHandle& defaultRenderPass = uniqueCameraData.m_RenderPasses[0];
		if (!defaultRenderPass) return;

		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		MaterialManager& materials = m_pModule->GetEngine()->GetMaterialManager();
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();
		AssetManager& assets = m_pModule->GetEngine()->GetAssetManager();

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
			const size_t totalBufferSize = finalPropertyDataSize * pipelineBatch.m_UniqueMaterials.size();
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
				if (std::memcmp(&batchData.m_MaterialDatas.m_Data[i * finalPropertyDataSize], buffer.data(), buffer.size()) != 0)
				{
					std::memcpy(&batchData.m_MaterialDatas.m_Data[i * finalPropertyDataSize], buffer.data(), buffer.size());
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
				batchData.m_WorldsBuffer = pDevice->CreateBuffer(batchData.m_Worlds->size() * sizeof(glm::mat4), BT_Storage, BF_Write);
				batchData.m_Worlds.SetDirty();
			}
			if (pDevice->BufferSize(batchData.m_WorldsBuffer) < batchData.m_Worlds.TotalByteSize())
				pDevice->ResizeBuffer(batchData.m_WorldsBuffer, batchData.m_Worlds.TotalByteSize());
			if (batchData.m_Worlds)
			{
				const size_t dirtySize = batchData.m_Worlds.DirtySize();
				pDevice->AssignBuffer(batchData.m_WorldsBuffer, batchData.m_Worlds.DirtyStart(),
					batchData.m_Worlds.m_DirtyRange.first * sizeof(glm::mat4), dirtySize * sizeof(glm::mat4));
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
				batchData.m_TextureBitsBuffer = pDevice->CreateBuffer(batchData.m_TextureBits->size() * sizeof(uint32_t), BT_Storage, BF_Write);
				batchData.m_TextureBits.SetDirty();
			}
			if (textureCount && pDevice->BufferSize(batchData.m_TextureBitsBuffer) < batchData.m_TextureBits.TotalByteSize())
				pDevice->ResizeBuffer(batchData.m_TextureBitsBuffer, batchData.m_TextureBits.TotalByteSize());
			if (textureCount && batchData.m_TextureBits)
			{
				const size_t dirtySize = batchData.m_TextureBits.DirtySize();
				pDevice->AssignBuffer(batchData.m_TextureBitsBuffer, batchData.m_TextureBits.DirtyStart(),
					batchData.m_TextureBits.m_DirtyRange.first * sizeof(uint32_t), dirtySize * sizeof(uint32_t));
			}

			if (!batchData.m_ObjectDataSet)
			{
				DescriptorSetInfo setInfo;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = batchData.m_WorldsBuffer;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = batchData.m_Worlds->size() * sizeof(glm::mat4);
				setInfo.m_Layout = RendererDSLayouts::m_ObjectDataSetLayout;
				batchData.m_ObjectDataSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}
			if (!batchData.m_MaterialSet)
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
					setInfo.m_Buffers[1].m_Size = batchData.m_TextureBits->size() * sizeof(uint32_t);
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
				dsWrite.m_Buffers[0].m_Size = batchData.m_Worlds->size() * sizeof(glm::mat4);
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
					dsWrite.m_Buffers[1].m_Size = batchData.m_TextureBits->size() * sizeof(uint32_t);
				}

				pDevice->UpdateDescriptorSet(batchData.m_MaterialSet, dsWrite);
			}

			if (!batchData.m_Pipeline || pPipelineData->IsDirty() || pPipelineData->SettingsDirty())
			{
				std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts(textureCount ? 7 : 6);
				descriptorSetLayouts[0] = RendererDSLayouts::m_GlobalRenderSetLayout;
				descriptorSetLayouts[1] = RendererDSLayouts::m_ObjectDataSetLayout;
				descriptorSetLayouts[2] = RendererDSLayouts::m_GlobalLightSetLayout;
				descriptorSetLayouts[3] = RendererDSLayouts::m_CameraLightSetLayout;
				descriptorSetLayouts[4] = batchData.m_MaterialSetLayout;
				descriptorSetLayouts[5] = RendererDSLayouts::m_ShadowAtlasSamplerSetLayout;
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

	void GloryRenderer::GenerateClusterSSBO(uint32_t cameraIndex, GraphicsDevice* pDevice, CameraRef camera, DescriptorSetHandle clusterSet)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::GenerateClusterSSBO" };
		const glm::uvec2 resolution = camera.GetResolution();

		ClusterConstants constants;
		constants.CameraIndex = cameraIndex;
		constants.LightCount = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		constants.GridSize = glm::vec4(GridSizeX, GridSizeY, NUM_DEPTH_SLICES, 0.0f);

		CommandBufferHandle commandBuffer = pDevice->Begin();
		pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_ClusterGeneratorPipeline);
		pDevice->BindDescriptorSets(commandBuffer, RendererPipelines::m_ClusterGeneratorPipeline, { m_GlobalClusterSet, clusterSet });
		pDevice->PushConstants(commandBuffer, RendererPipelines::m_ClusterGeneratorPipeline, 0, sizeof(ClusterConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
		pDevice->Dispatch(commandBuffer, constants.GridSize.x, constants.GridSize.y, constants.GridSize.z);
		pDevice->EndPipeline(commandBuffer);
		pDevice->End(commandBuffer);
		pDevice->Commit(commandBuffer);
		pDevice->Wait(commandBuffer);
		pDevice->Release(commandBuffer);
	}

	void GloryRenderer::PrepareLineMesh(GraphicsDevice* pDevice)
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
			m_LineBuffers[m_CurrentFrameIndex] = pDevice->CreateBuffer(m_LineVertices->size() * sizeof(LineVertex), BT_Vertex, BF_None);
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

	void GloryRenderer::PrepareSkybox(GraphicsDevice* pDevice)
	{
		GScene* pActiveScene = m_pModule->GetEngine()->GetSceneManager()->GetActiveScene();
		if (!pActiveScene) return;
		const UUID skyboxID = pActiveScene->Settings().m_LightingSettings.m_Skybox;
		Resource* pResource = m_pModule->GetEngine()->GetAssetManager().FindResource(skyboxID);
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
			m_GlobalSkyboxSamplerSet = CreateSamplerDescriptorSet(pDevice, 1, { cubemap }, RendererDSLayouts::m_GlobalSkyboxSamplerSetLayout);
			m_SkyboxCubemap = cubemap;
			return;
		}
		if (!cubemapDirty && cubemap == m_SkyboxCubemap) return;

		DescriptorSetUpdateInfo dsUpdateInfo;
		dsUpdateInfo.m_Samplers = { { cubemap, 0 } };
		pDevice->UpdateDescriptorSet(m_GlobalSkyboxSamplerSet, dsUpdateInfo);
		m_SkyboxCubemap = cubemap;
	}

	void GloryRenderer::ClusterPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::ClusterPass" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();

		CameraRef camera = m_ActiveCameras[cameraIndex];
		const UniqueCameraData& uniqueCameraData = m_UniqueCameraDatas.at(camera.GetUUID());
		const DescriptorSetHandle& clusterSet = uniqueCameraData.m_ClusterSet;
		const DescriptorSetHandle& lightSet = uniqueCameraData.m_LightSets[m_CurrentFrameIndex];
		const DescriptorSetHandle& lightDistancesSet = m_LightDistancesSets[m_CurrentFrameIndex];

		const glm::uvec2 resolution = camera.GetResolution();

		ClusterConstants constants;
		constants.CameraIndex = cameraIndex;
		constants.LightCount = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
		constants.GridSize = glm::vec4(GridSizeX, GridSizeY, NUM_DEPTH_SLICES, 0.0f);

		pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_ClusterCullLightPipeline);
		pDevice->BindDescriptorSets(commandBuffer, RendererPipelines::m_ClusterCullLightPipeline, { m_GlobalClusterSet, clusterSet, m_GlobalLightSet, lightSet, lightDistancesSet });
		pDevice->PushConstants(commandBuffer, RendererPipelines::m_ClusterCullLightPipeline, 0, sizeof(ClusterConstants), &constants, ShaderTypeFlag(STF_Fragment | STF_Compute));
		pDevice->Dispatch(commandBuffer, 1, 1, 6);
		pDevice->EndPipeline(commandBuffer);
	}

	void GloryRenderer::SkyboxPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex)
	{
		GScene* pActiveScene = m_pModule->GetEngine()->GetSceneManager()->GetActiveScene();
		if (!pActiveScene) return;
		const UUID skyboxID = pActiveScene->Settings().m_LightingSettings.m_Skybox;
		if (!skyboxID) return;
		Resource* pResource = m_pModule->GetEngine()->GetAssetManager().FindResource(skyboxID);
		if (!pResource) return;

		CameraRef camera = m_ActiveCameras[cameraIndex];
		const glm::uvec2& resolution = camera.GetResolution();

		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::SkyboxPass" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_SkyboxPipeline);
		pDevice->BindDescriptorSets(commandBuffer, RendererPipelines::m_SkyboxPipeline, { m_GlobalSkyboxRenderSet, m_GlobalSkyboxSamplerSet });
		pDevice->PushConstants(commandBuffer, RendererPipelines::m_SkyboxPipeline, 0, sizeof(uint32_t), &cameraIndex, STF_Vertex);
		pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
		pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
		pDevice->DrawUnitCube(commandBuffer);
		pDevice->EndPipeline(commandBuffer);
	}

	void GloryRenderer::DynamicObjectsPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::DynamicObjectsPass" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		CameraRef camera = m_ActiveCameras[cameraIndex];
		DescriptorSetHandle shadowAtlasSet = m_ShadowAtlasSamplerSets[m_CurrentFrameIndex];
		RenderBatches(commandBuffer, m_DynamicPipelineRenderDatas, m_DynamicBatchData, cameraIndex, m_GlobalRenderSet,
			{ 0.0f, 0.0f, camera.GetResolution() }, shadowAtlasSet);
	}

	void GloryRenderer::DynamicLateObjectsPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::DynamicLateObjectsPass" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		CameraRef camera = m_ActiveCameras[cameraIndex];
		DescriptorSetHandle shadowAtlasSet = m_ShadowAtlasSamplerSets[m_CurrentFrameIndex];
		RenderBatches(commandBuffer, m_DynamicLatePipelineRenderDatas, m_DynamicLateBatchData, cameraIndex, m_GlobalRenderSet,
			{ 0.0f, 0.0f, camera.GetResolution() }, shadowAtlasSet);
	}

	std::uniform_real_distribution<float> RandomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine NumberGenerator;

	void GloryRenderer::GenerateDomeSamplePointsSSBO(GraphicsDevice* pDevice, uint32_t size)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::GenerateDomeSamplePointsSSBO" };
		if (m_SSAOKernelSize == size) return;
		m_SSAOKernelSize = size;

		if (!m_SamplePointsDomeSSBO)
			m_SamplePointsDomeSSBO = pDevice->CreateBuffer(sizeof(glm::vec3) * MAX_KERNEL_SIZE, BufferType::BT_Uniform, BF_CopyDst);

		std::vector<glm::vec3> samplePoints{ m_SSAOKernelSize, glm::vec3{} };
		for (unsigned int i = 0; i < m_SSAOKernelSize; ++i)
		{
			samplePoints[i] = glm::vec3{
				RandomFloats(NumberGenerator) * 2.0 - 1.0,
				RandomFloats(NumberGenerator) * 2.0 - 1.0,
				RandomFloats(NumberGenerator)
			};
			samplePoints[i] = glm::normalize(samplePoints[i]);
			samplePoints[i] *= RandomFloats(NumberGenerator);

			float scale = float(i) / m_SSAOKernelSize;
			scale = lerp(0.1f, 1.0f, scale * scale);
			samplePoints[i] *= scale;
		}

		pDevice->AssignBuffer(m_SamplePointsDomeSSBO, samplePoints.data(), 0, sizeof(glm::vec3) * m_SSAOKernelSize);
	}

	void GloryRenderer::GenerateNoiseTexture(GraphicsDevice* pDevice)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::GenerateNoiseTexture" };
		const size_t textureSize = 4;
		std::vector<glm::vec4> ssaoNoise;
		for (unsigned int i = 0; i < textureSize * textureSize; ++i)
		{
			glm::vec4 noise(
				RandomFloats(NumberGenerator) * 2.0 - 1.0,
				RandomFloats(NumberGenerator) * 2.0 - 1.0,
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
		m_SampleNoiseTexture = pDevice->CreateTexture(textureInfo, static_cast<const void*>(ssaoNoise.data()), sizeof(glm::vec4) * ssaoNoise.size());
	}

	void GloryRenderer::ShadowMapsPass(CommandBufferHandle commandBuffer)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::ShadowMapsPass" };
		if (m_FrameData.ActiveLights.count() == 0) return;

		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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

	void GloryRenderer::RenderShadows(CommandBufferHandle commandBuffer, size_t lightIndex, const glm::vec4& viewport)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::RenderShadows" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
		MaterialManager& materialManager = m_pModule->GetEngine()->GetMaterialManager();
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();
		AssetManager& assets = m_pModule->GetEngine()->GetAssetManager();

		RenderConstants constants;
		constants.m_CameraIndex = static_cast<uint32_t>(lightIndex);
		constants.m_LightCount = m_FrameData.ActiveLights.count();

		pDevice->BeginPipeline(commandBuffer, RendererPipelines::m_ShadowRenderPipeline);

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

	void GloryRenderer::OnSubmitCamera(CameraRef camera)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::OnSubmitCamera" };
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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
				dsInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
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
				dsInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
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
				lightIndexSSBO = pDevice->CreateBuffer(sizeof(uint32_t) * (NUM_CLUSTERS * MAX_LIGHTS_PER_TILE + 1), BufferType::BT_Storage, BF_None);
			if (!lightGridSSBO)
				lightGridSSBO = pDevice->CreateBuffer(sizeof(LightGrid) * NUM_CLUSTERS, BufferType::BT_Storage, BF_None);
			if (!pickResultsUBO)
				pickResultsUBO = pDevice->CreateBuffer(sizeof(GPUPickResult) * MaxPicks + sizeof(uint32_t) * 4, BufferType::BT_Storage, BF_Read);

			if (!lightSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_CameraLightSetLayout;
				setInfo.m_Buffers.resize(2);
				setInfo.m_Buffers[0].m_BufferHandle = lightIndexSSBO;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(uint32_t) * (NUM_CLUSTERS * MAX_LIGHTS_PER_TILE + 1);
				setInfo.m_Buffers[1].m_BufferHandle = lightGridSSBO;
				setInfo.m_Buffers[1].m_Offset = 0;
				setInfo.m_Buffers[1].m_Size = sizeof(LightGrid) * NUM_CLUSTERS;
				lightSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!ssaoSamplersSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_SSAOSamplersSetLayout;
				setInfo.m_Samplers.resize(2);
				setInfo.m_Samplers[0].m_TextureHandle = normals;
				setInfo.m_Samplers[1].m_TextureHandle = depth;
				ssaoSamplersSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!ssaoPostSamplersSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_SSAOPostSamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = ao;
				ssaoPostSamplersSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!colorSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = color;
				colorSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!pickingResultsSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_PickingResultSetLayout;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = pickResultsUBO;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(GPUPickResult) * MaxPicks + sizeof(uint32_t) * 4;
				pickingResultsSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!pickingSamplersSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_PickingSamplerSetLayout;
				setInfo.m_Samplers.resize(3);
				setInfo.m_Samplers[0].m_TextureHandle = objectID;
				setInfo.m_Samplers[1].m_TextureHandle = normals;
				setInfo.m_Samplers[2].m_TextureHandle = depth;
				pickingSamplersSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!objectIDSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = objectID;
				objectIDSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!normalSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = normals;
				normalSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!aoSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = ao;
				aoSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!depthSamplerSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_DisplayCopySamplerSetLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = depth;
				depthSamplerSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}

			if (!lightGridSet)
			{
				DescriptorSetInfo setInfo = DescriptorSetInfo();
				setInfo.m_Layout = RendererDSLayouts::m_LightGridSetLayout;
				setInfo.m_Buffers.resize(1);
				setInfo.m_Buffers[0].m_BufferHandle = lightGridSSBO;
				setInfo.m_Buffers[0].m_Offset = 0;
				setInfo.m_Buffers[0].m_Size = sizeof(LightGrid) * NUM_CLUSTERS;
				lightGridSet = pDevice->CreateDescriptorSet(std::move(setInfo));
			}
		}
	}

	void GloryRenderer::OnUnsubmitCamera(CameraRef camera)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::OnUnsubmitCamera" };
	}

	void GloryRenderer::OnCameraUpdated(CameraRef camera)
	{
		ProfileSample s{ &m_pModule->GetEngine()->Profiler(), "GloryRenderer::OnCameraUpdated" };
	}

	void GloryRenderer::PrepareCameras()
	{
		GraphicsDevice* pDevice = m_pModule->GetEngine()->ActiveGraphicsDevice();
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
				camera.GetNear(), camera.GetFar(), { static_cast<glm::vec2>(camera.GetResolution()) } };

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
				m_CameraDatas.m_DirtyRange.first * sizeof(PerCameraData), static_cast<uint32_t>(dirtySize * sizeof(PerCameraData)));
		}
	}

	void GloryRenderer::GenerateShadowMapLODResolutions()
	{
		m_ShadowMapResolutions.reserve(m_MaxShadowLODs);
		for (size_t i = 0; i < m_MaxShadowLODs; ++i)
		{
			const uint32_t res = std::max(m_MinShadowResolution, m_MaxShadowResolution / m_ShadowLODDivisions[i]);
			m_ShadowMapResolutions.push_back({ res, res });
		}
	}

	void GloryRenderer::ResizeShadowMapLODResolutions(uint32_t minSize, uint32_t maxSize)
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

	void GloryRenderer::GenerateShadowLODDivisions(uint32_t maxLODs)
	{
		m_MaxShadowLODs = std::min(RendererCVARs::MAX_SHADOW_LODS, maxLODs);
		m_ShadowLODDivisions.clear();

		m_ShadowLODDivisions.reserve(m_MaxShadowLODs);
		for (size_t i = 0; i < m_MaxShadowLODs; ++i)
		{
			const uint32_t divider = uint32_t(pow(2, i));
			m_ShadowLODDivisions.push_back(divider);
		}
	}

	void GloryRenderer::SetPipelineOrder(std::vector<UUID>&& pipelineOrder)
	{
		m_pModule->SetPipelineOrder(std::move(pipelineOrder));
	}
}
