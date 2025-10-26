#pragma once
#include <RendererModule.h>
#include <MaterialData.h>
#include <FileData.h>

#include <glm/glm.hpp>

namespace Glory
{
	class GraphicsDevice;
	class GPUTextureAtlas;

	struct VolumeTileAABB
	{
		glm::vec4 MinPoint;
		glm::vec4 MaxPoint;
	};

	struct LightGrid
	{
		uint32_t Offset;
		uint32_t Count;
	};

	struct ClusterConstants
	{
		glm::uvec4 GridSize;
		uint32_t LightCount;
		uint32_t CameraIndex;
	};

	struct SSAOConstants
	{
		uint32_t CameraIndex;
		int32_t KernelSize;
		float SampleRadius;
		float SampleBias;
	};

	struct RenderConstants
	{
		glm::uvec4 m_GridSize;
		UUID m_SceneID;
		UUID m_ObjectID;
		uint32_t m_ObjectDataIndex;
		uint32_t m_CameraIndex;
		uint32_t m_MaterialIndex;
		uint32_t m_LightCount;
	};

	struct PickingConstants
	{
		uint32_t m_CameraIndex;
		uint32_t m_NumPickings;
		uint32_t m_Padding1;
		uint32_t m_Padding2;
		glm::ivec2 m_Picks[16];
	};

	/*struct DeferredConstants
	{
		glm::uvec4 TileSizes;
		uint64_t HasTexture;
		uint32_t CameraIndex;
		float Scale;
		float Bias;
		int32_t AOEnabled;
		float AOMagnitude;
		float AOContrast;
	};*/

	class GloryRendererModule : public RendererModule
	{
	private:
		struct PipelineBatchData
		{
			CPUBuffer<glm::mat4> m_Worlds;
			BufferHandle m_WorldsBuffer = 0;

			std::vector<uint32_t> m_MaterialIndices;
			CPUBuffer<char> m_MaterialDatas;
			CPUBuffer<uint32_t> m_TextureBits;
			BufferHandle m_MaterialsBuffer = 0;
			BufferHandle m_TextureBitsBuffer = 0;

			PipelineHandle m_Pipeline = 0;
			DescriptorSetLayoutHandle m_TextureSetLayout = 0;
			DescriptorSetLayoutHandle m_MaterialSetLayout = 0;
			DescriptorSetHandle m_ObjectDataSet = 0;
			DescriptorSetHandle m_MaterialSet = 0;
			std::vector<DescriptorSetHandle> m_TextureSets;
		};

		struct GPUPickResult
		{
			UUID SceneID;
			UUID ObjectID;
			glm::vec4 Normal;
			glm::vec4 Position;
		};

	public:
		GloryRendererModule();
		virtual ~GloryRendererModule();

		virtual void OnCameraResize(CameraRef camera) override;
		virtual void OnCameraPerspectiveChanged(CameraRef camera) override;
		virtual MaterialData* GetInternalMaterial(std::string_view name) const override;

		virtual void CollectReferences(std::vector<UUID>& references) override;

		virtual UUID TextPipelineID() const override;
		virtual void PresentFrame() override;

		GLORY_MODULE_VERSION_H(0, 1, 0);

	private:
		virtual void Cleanup() override;
		virtual void Initialize() override;
		virtual void OnPostInitialize() override;
		virtual void Update() override;
		virtual void Draw() override;
		virtual void LoadSettings(ModuleSettings& settings) override;

		virtual size_t DefaultAttachmenmtIndex() const override;
		virtual size_t CameraAttachmentPreviewCount() const override;
		virtual std::string_view CameraAttachmentPreviewName(size_t index) const override;
		virtual TextureHandle CameraAttachmentPreview(CameraRef camera, size_t index) const override;
		virtual TextureHandle FinalColor() const override;

		virtual size_t DebugOverlayCount() const override;
		virtual std::string_view DebugOverlayName(size_t index) const override;
		virtual TextureHandle DebugOverlay(size_t index) const override;

		virtual void OnWindowResized() override;
		virtual void OnSwapchainChanged() override;

	private:
		size_t GetGCD(size_t a, size_t b); // TODO: Move this to somewhere it can be used from anywhere and make it take templates

		void RenderBatches(CommandBufferHandle commandBuffer, const std::vector<PipelineBatch>& batches,
			const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex, DescriptorSetHandle globalRenderSet, const glm::vec4& viewport,
			DescriptorSetHandle shadowsSet);
		void PrepareDataPass();
		void PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas);
		void GenerateClusterSSBO(uint32_t cameraIndex, GraphicsDevice* pDevice, CameraRef camera, DescriptorSetHandle clusterSet);

		void ClusterPass(CommandBufferHandle commandBuffer, uint32_t cameraIndex);
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
		std::vector<PipelineBatchData> m_DynamicBatchData;
		std::vector<PipelineBatchData> m_DynamicLateBatchData;
		CPUBuffer<PerCameraData> m_CameraDatas;
		CPUBuffer<PerCameraData> m_LightCameraDatas;

		/* Buffers */
		BufferHandle m_CameraDatasBuffer = 0;
		BufferHandle m_LightCameraDatasBuffer = 0;
		BufferHandle m_RenderConstantsBuffer = 0;
		BufferHandle m_PickingConstantsBuffer = 0;
		BufferHandle m_ClusterConstantsBuffer = 0;
		BufferHandle m_DeferredConstantsBuffer = 0;
		BufferHandle m_LightsSSBO = 0;
		BufferHandle m_LightSpaceTransformsSSBO = 0;
		BufferHandle m_SSAOConstantsBuffer = 0;
		BufferHandle m_SamplePointsDomeSSBO = 0;

		/* Descriptor set layouts */
		DescriptorSetLayoutHandle m_GlobalRenderSetLayout;
		DescriptorSetLayoutHandle m_GlobalShadowRenderSetLayout;
		DescriptorSetLayoutHandle m_GlobalPickingSetLayout;
		DescriptorSetLayoutHandle m_GlobalClusterSetLayout;
		DescriptorSetLayoutHandle m_GlobalLightSetLayout;
		DescriptorSetLayoutHandle m_GlobalSampleDomeSetLayout;
		DescriptorSetLayoutHandle m_CameraClusterSetLayout;
		DescriptorSetLayoutHandle m_CameraLightSetLayout;
		DescriptorSetLayoutHandle m_SSAOSamplersSetLayout;
		DescriptorSetLayoutHandle m_NoiseSamplerSetLayout;
		DescriptorSetLayoutHandle m_CameraSamplerSetLayout;
		DescriptorSetLayoutHandle m_ShadowAtlasSamplerSetLayout;
		DescriptorSetLayoutHandle m_ObjectDataSetLayout;
		DescriptorSetLayoutHandle m_LightDistancesSetLayout;
		DescriptorSetLayoutHandle m_PickingResultSetLayout;
		DescriptorSetLayoutHandle m_PickingSamplerSetLayout;

		DescriptorSetLayoutHandle m_DisplayCopySamplerSetLayout;

		/* Descriptor sets */
		DescriptorSetHandle m_GlobalRenderSet;
		DescriptorSetHandle m_GlobalShadowRenderSet;
		DescriptorSetHandle m_GlobalPickingSet;
		DescriptorSetHandle m_GlobalClusterSet;
		DescriptorSetHandle m_GlobalDeferredSet;
		DescriptorSetHandle m_GlobalLightSet;
		DescriptorSetHandle m_GlobalSampleDomeSet;
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

		/* Compute pipelines */
		PipelineHandle m_ClusterGeneratorPipeline = 0;
		PipelineHandle m_ClusterCullLightPipeline = 0;
		PipelineHandle m_PickingPipeline = 0;

		/* Effects pipelines */
		PipelineHandle m_SSAOPipeline = 0;

		/* Shadow rendering */
		PipelineHandle m_ShadowRenderPipeline = 0;
		PipelineHandle m_TransparentShadowRenderPipeline = 0;

		/* SSAO */
		SSAOSettings m_GlobalSSAOSetting;
		uint32_t m_SSAOKernelSize = 0;

		/* Shadows */
		std::vector<RenderPassHandle> m_ShadowsPasses;
		std::vector<size_t> m_ShadowAtlasses;

		std::vector<CameraRef> m_DirtyCameraPerspectives;

		std::vector<RenderPassHandle> m_SwapchainPasses;
		std::vector<SemaphoreHandle> m_ImageAvailableSemaphores;
		std::vector<SemaphoreHandle> m_RenderingFinishedSemaphores;
		uint32_t m_CurrentSemaphoreIndex = 0;
		uint32_t m_CurrentFrameIndex = 0;
		uint32_t m_ImageCount = 1;

		struct UniqueCameraData
		{
			BufferHandle m_ClusterSSBO = 0;
			DescriptorSetHandle m_ClusterSet = 0;

			std::vector<RenderPassHandle> m_RenderPasses;
			std::vector<RenderPassHandle> m_LateRenderPasses;
			std::vector<RenderPassHandle> m_SSAORenderPasses;
			std::vector<BufferHandle> m_LightIndexSSBOs;
			std::vector<BufferHandle> m_LightGridSSBOs;
			std::vector<BufferHandle> m_PickResultsSSBOs;

			std::vector<DescriptorSetHandle> m_LightSets;
			std::vector<DescriptorSetHandle> m_SSAOSamplersSets;
			std::vector<DescriptorSetHandle> m_FinalColorSamplerSets;
			std::vector<DescriptorSetHandle> m_PickingResultSets;
			std::vector<DescriptorSetHandle> m_PickingSamplersSets;

			std::vector<glm::ivec2> m_Picks;
		};
		std::vector<BufferHandle> m_LightDistancesSSBOs;
		std::vector<uint32_t> m_ClosestLightDepthSlices;
		std::vector<DescriptorSetHandle> m_LightDistancesSets;

		std::vector<DescriptorSetHandle> m_ShadowAtlasSamplerSets;

		std::map<UUID, UniqueCameraData> m_UniqueCameraDatas;
		std::vector<CommandBufferHandle> m_FrameCommandBuffers;

		std::vector<RenderPassHandle> m_FinalFrameColorPasses;
		std::vector<DescriptorSetHandle> m_FinalFrameColorSets;
		PipelineHandle m_DisplayCopyPipeline = 0;

		uint32_t m_MinShadowResolution;
		uint32_t m_MaxShadowResolution;
		uint32_t m_ShadowAtlasResolution;
		uint32_t m_MaxShadowLODs;
		static const uint32_t MAX_SHADOW_LODS = 24;
		std::vector<uint32_t> m_ShadowLODDivisions;
		std::vector<glm::uvec2> m_ShadowMapResolutions;
	};
}
