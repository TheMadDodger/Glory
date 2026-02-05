#pragma once
#include "GloryRendererData.h"

#include <Renderer.h>
#include <GraphicsEnums.h>

namespace Glory
{
	class Engine;
	class GraphicsDevice;
	class GPUTextureAtlas;
	class GloryRendererModule;

	struct PipelineBatchData
	{
		CPUBuffer<glm::mat4> m_Worlds;
		BufferHandle m_WorldsBuffer = 0;

		std::vector<uint32_t> m_MaterialIndices;
		CPUBuffer<char> m_MaterialDatas;
		CPUBuffer<uint32_t> m_TextureBits;
		BufferHandle m_MaterialsBuffer = 0;
		BufferHandle m_TextureBitsBuffer = 0;

		std::vector<UUID> m_LastFrameUniqueMaterials;

		PipelineHandle m_Pipeline = 0;
		DescriptorSetLayoutHandle m_TextureSetLayout = 0;
		DescriptorSetLayoutHandle m_MaterialSetLayout = 0;
		DescriptorSetHandle m_ObjectDataSet = 0;
		DescriptorSetHandle m_MaterialSet = 0;
		std::vector<DescriptorSetHandle> m_TextureSets;
	};

	struct UniqueCameraData
	{
		BufferHandle m_ClusterSSBO = 0;
		DescriptorSetHandle m_ClusterSet = 0;

		std::vector<RenderPassHandle> m_RenderPasses;
		std::vector<RenderPassHandle> m_LateRenderPasses;
		std::vector<RenderPassHandle> m_SSAORenderPasses;
		std::vector<PostProcessPass> m_PostProcessPasses;
		std::vector<BufferHandle> m_LightIndexSSBOs;
		std::vector<BufferHandle> m_LightGridSSBOs;
		std::vector<BufferHandle> m_PickResultsSSBOs;

		std::vector<DescriptorSetHandle> m_LightSets;
		std::vector<DescriptorSetHandle> m_SSAOSamplersSets;
		std::vector<DescriptorSetHandle> m_SSAOPostSamplersSets;
		std::vector<DescriptorSetHandle> m_PickingResultSets;
		std::vector<DescriptorSetHandle> m_PickingSamplersSets;

		std::vector<DescriptorSetHandle> m_ObjectIDSamplerSets;
		std::vector<DescriptorSetHandle> m_ColorSamplerSets;
		std::vector<DescriptorSetHandle> m_NormalSamplerSets;
		std::vector<DescriptorSetHandle> m_AOSamplerSets;
		std::vector<DescriptorSetHandle> m_DepthSamplerSets;

		std::vector<DescriptorSetHandle> m_LightGridSets;

		std::vector<glm::ivec2> m_Picks;

		CameraAttachment m_VisualizedAttachment;
		Utils::BitSet m_DebugOverlayBits;
	};

	class GloryRenderer : public Renderer
	{
	public:
		GloryRenderer(GloryRendererModule* pModule);
		virtual ~GloryRenderer();

		virtual void OnCameraResize(CameraRef camera) override;
		virtual void OnCameraPerspectiveChanged(CameraRef camera) override;
		virtual MaterialData* GetInternalMaterial(std::string_view name) const override;

		virtual void PresentFrame() override;

		virtual uint32_t GetNumFramesInFlight() const override;
		virtual uint32_t GetCurrentFrameInFlight() const override;

		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void InitializeAsMainRenderer() override;
		virtual void Draw() override;

	private:
		virtual size_t DefaultAttachmenmtIndex() const override;
		virtual size_t CameraAttachmentPreviewCount() const override;
		virtual std::string_view CameraAttachmentPreviewName(size_t index) const override;
		virtual TextureHandle CameraAttachmentPreview(CameraRef camera, size_t index) const override;
		virtual TextureHandle FinalColor() const override;
		virtual void VisualizeAttachment(CameraRef camera, size_t index) override;

		virtual size_t DebugOverlayCount() const override;
		virtual std::string_view DebugOverlayName(size_t index) const override;
		virtual void SetDebugOverlayEnabled(CameraRef camera, size_t index, bool enabled = true) override;
		virtual bool DebugOverlayEnabled(CameraRef camera, size_t index) const override;

		virtual void OnWindowResized() override;
		virtual void OnSwapchainChanged() override;
		virtual RenderPassHandle GetSwapchainPass() const override;
		virtual RenderPassHandle GetDummyPostProcessPass() const override;

	private:
		size_t GetGCD(size_t a, size_t b); // TODO: Move this to somewhere it can be used from anywhere and make it take templates

		void RenderBatches(CommandBufferHandle commandBuffer, const std::vector<PipelineBatch>& batches,
			const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex, DescriptorSetHandle globalRenderSet, const glm::vec4& viewport,
			DescriptorSetHandle shadowsSet);
		void PrepareDataPass();
		void PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas);
		void GenerateClusterSSBO(uint32_t cameraIndex, GraphicsDevice* pDevice, CameraRef camera, DescriptorSetHandle clusterSet);
		void PrepareLineMesh(GraphicsDevice* pDevice);
		void PrepareSkybox(GraphicsDevice* pDevice);

		void ClusterPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex);
		void SkyboxPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex);
		void DynamicObjectsPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex);
		void DynamicLateObjectsPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex);

		void GenerateDomeSamplePointsSSBO(GraphicsDevice* pDevice, uint32_t size);
		void GenerateNoiseTexture(GraphicsDevice* pDevice);

		void ShadowMapsPass(CommandBufferHandle commandBuffer);
		void RenderShadows(CommandBufferHandle commandBuffer, size_t lightIndex, const glm::vec4& viewport);

		virtual void OnSubmitCamera(CameraRef camera) override;
		virtual void OnUnsubmitCamera(CameraRef camera) override;
		virtual void OnCameraUpdated(CameraRef camera) override;

		void PrepareCameras();

		void GenerateShadowMapLODResolutions();
		void ResizeShadowMapLODResolutions(uint32_t minSize, uint32_t maxSize);
		void GenerateShadowLODDivisions(uint32_t maxLODs);

	private:
		friend class GloryRendererModule;
		GloryRendererModule* m_pModule;

		std::vector<PipelineBatchData> m_DynamicBatchData;
		std::vector<PipelineBatchData> m_DynamicLateBatchData;
		CPUBuffer<PerCameraData> m_CameraDatas;
		CPUBuffer<PerCameraData> m_LightCameraDatas;
		CPUBuffer<glm::mat4> m_LightSpaceTransforms;

		/* Buffers */
		BufferHandle m_CameraDatasBuffer = 0;
		BufferHandle m_LightCameraDatasBuffer = 0;
		BufferHandle m_LightsSSBO = 0;
		BufferHandle m_LightSpaceTransformsSSBO = 0;
		BufferHandle m_SamplePointsDomeSSBO = 0;

		/* Descriptor sets */
		/* Global */
		DescriptorSetHandle m_GlobalRenderSet;
		DescriptorSetHandle m_GlobalShadowRenderSet;
		DescriptorSetHandle m_GlobalPickingSet;
		DescriptorSetHandle m_GlobalSkyboxRenderSet;
		DescriptorSetHandle m_GlobalSkyboxSamplerSet = 0;
		DescriptorSetHandle m_GlobalLineRenderSet;
		DescriptorSetHandle m_GlobalClusterSet;
		DescriptorSetHandle m_GlobalDeferredSet;
		DescriptorSetHandle m_GlobalLightSet;
		DescriptorSetHandle m_GlobalSampleDomeSet;

		/* Individual */
		DescriptorSetHandle m_SSAOCameraSet;
		DescriptorSetHandle m_NoiseSamplerSet;

		static const size_t m_GridSizeX = 16;
		static const size_t m_GridSizeY = 9;
		static const size_t NUM_DEPTH_SLICES = 24;
		static const size_t NUM_CLUSTERS = m_GridSizeX * m_GridSizeY * NUM_DEPTH_SLICES;
		static const size_t MAX_LIGHTS_PER_TILE = 50;
		static const size_t MAX_KERNEL_SIZE = 1024;

		/* Textures */
		TextureHandle m_SampleNoiseTexture = 0;
		TextureHandle m_SkyboxCubemap = 0;

		/* SSAO */
		SSAOSettings m_GlobalSSAOSetting;
		uint32_t m_SSAOKernelSize = 0;

		/* Shadows */
		std::vector<RenderPassHandle> m_ShadowsPasses;
		std::vector<size_t> m_ShadowAtlasses;

		uint32_t m_MinShadowResolution = 256;
		uint32_t m_MaxShadowResolution = 2048;
		uint32_t m_ShadowAtlasResolution = 8192;
		uint32_t m_MaxShadowLODs = 6;
		std::vector<uint32_t> m_ShadowLODDivisions;
		std::vector<glm::uvec2> m_ShadowMapResolutions;

		std::vector<CameraRef> m_DirtyCameraPerspectives;

		std::vector<RenderPassHandle> m_SwapchainPasses;
		std::vector<SemaphoreHandle> m_ImageAvailableSemaphores;
		std::vector<SemaphoreHandle> m_RenderingFinishedSemaphores;
		uint32_t m_CurrentSemaphoreIndex = 0;
		uint32_t m_CurrentFrameIndex = 0;
		uint32_t m_ImageCount = 1;

		std::vector<BufferHandle> m_LightDistancesSSBOs;
		std::vector<uint32_t> m_ClosestLightDepthSlices;
		std::vector<DescriptorSetHandle> m_LightDistancesSets;

		std::vector<DescriptorSetHandle> m_ShadowAtlasSamplerSets;

		std::map<UUID, UniqueCameraData> m_UniqueCameraDatas;
		std::vector<CommandBufferHandle> m_FrameCommandBuffers;

		std::vector<RenderPassHandle> m_FinalFrameColorPasses;
		std::vector<DescriptorSetHandle> m_FinalFrameColorSets;

		CPUBuffer<LineVertex> m_LineVertices;
		std::vector<BufferHandle> m_LineBuffers;
		std::vector<MeshHandle> m_LineMeshes;

		Utils::BitSet m_DebugOverlayBits;
	};
}