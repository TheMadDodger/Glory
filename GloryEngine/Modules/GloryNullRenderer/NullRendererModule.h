#pragma once
#include <RendererModule.h>
#include <Material.h>
#include <MaterialData.h>
#include <FileData.h>
#include <glm/glm.hpp>
#include <vector>

namespace Glory
{
	typedef struct UUID MeshHandle;
	typedef struct UUID PipelineHandle;
	typedef struct UUID BufferHandle;
	typedef struct UUID TextureHandle;
	typedef struct UUID RenderTextureHandle;
	typedef struct UUID RenderPassHandle;
	typedef struct UUID ShaderHandle;
	typedef struct UUID DescriptorSetHandle;
	typedef struct UUID DescriptorSetLayoutHandle;

	class PipelineData;
	class TextureAtlas;
	class GraphicsDevice;

	struct VolumeTileAABB
	{
		glm::vec4 MinPoint;
		glm::vec4 MaxPoint;
	};

	struct ScreenToView
	{
		glm::mat4 ProjectionInverse;
		glm::mat4 ViewInverse;
		glm::uvec4 TileSizes;
		glm::uvec2 ScreenDimensions;
		float Scale;
		float Bias;
		float zNear;
		float zFar;
	};

	struct LightGrid
	{
		uint32_t Offset;
		uint32_t Count;
	};

	class NullRendererModule : public RendererModule
	{
	public:
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
			DescriptorSetLayoutHandle m_SetLayout = 0;
			DescriptorSetLayoutHandle m_TextureSetLayout = 0;
			DescriptorSetHandle m_Set = 0;
			std::vector<DescriptorSetHandle> m_TextureSets;
		};

		NullRendererModule();
		virtual ~NullRendererModule();

		virtual void GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos) override;
		virtual void OnCameraResize(CameraRef camera) override;
		virtual void OnCameraPerspectiveChanged(CameraRef camera) override;
		virtual MaterialData* GetInternalMaterial(std::string_view name) const override;

		virtual void CollectReferences(std::vector<UUID>& references) override;

		virtual UUID TextPipelineID() const override;

		GLORY_MODULE_VERSION_H(0, 1, 0);

	private:
		virtual void Cleanup() override;
		virtual void Initialize() override;
		virtual void OnPostInitialize() override;
		virtual void Update() override;

		virtual void OnRenderEffects(CameraRef camera, RenderTexture* pRenderTexture) override;
		virtual void OnDoCompositing(CameraRef camera, uint32_t width, uint32_t height, RenderTexture* pRenderTexture) override;
		virtual void OnDisplayCopy(RenderTexture* pRenderTexture, uint32_t width, uint32_t height) override;
		virtual void OnRenderSkybox(CameraRef camera, CubemapData* pCubemap) override;

		virtual void OnStartCameraRender(CameraRef camera, const FrameData<LightData>& lights) override;
		virtual void OnEndCameraRender(CameraRef camera, const FrameData<LightData>& lights) override;

		virtual void LoadSettings(ModuleSettings& settings) override;

		void RenderBatches(const std::vector<PipelineBatch>& batches, const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex);
		void PrepareDataPass();
		void PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas);

		void ClusterPass(uint32_t cameraIndex);
		void DynamicObjectsPass(uint32_t cameraIndex);

		void GenerateClusterSSBO(GraphicsDevice* pDevice, CameraRef camera, DescriptorSetHandle clusterSet);

	private:
		std::vector<PipelineBatchData> m_DynamicBatchData;
		CPUBuffer<PerCameraData> m_CameraDatas;
		CPUBuffer<PerCameraData> m_LightCameraDatas;
		BufferHandle m_CameraDatasBuffer = 0;
		//BufferHandle m_LightCameraDatasBuffer = 0;
		BufferHandle m_RenderConstantsBuffer = 0;
		BufferHandle m_ScreenToViewBuffer = 0;

		DescriptorSetHandle m_GlobalSet;
		DescriptorSetLayoutHandle m_GlobalSetLayout;

		BufferHandle m_LightsSSBO = 0;
		BufferHandle m_LightCountSSBO = 0;
		//BufferHandle m_LightSpaceTransformsSSBO = 0;
		BufferHandle m_LightDistancesSSBO = 0;

		static const size_t m_GridSizeX = 16;
		static const size_t m_GridSizeY = 9;
		static const size_t NUM_DEPTH_SLICES = 24;
		static const size_t NUM_CLUSTERS = m_GridSizeX * m_GridSizeY * NUM_DEPTH_SLICES;
		static const size_t MAX_LIGHTS_PER_TILE = 50;
		static const size_t MAX_KERNEL_SIZE = 1024;

		/* Compute shaders */
		DescriptorSetLayoutHandle m_ClusterSetLayout;
		DescriptorSetLayoutHandle m_ClusterCullLightSetLayout;
		PipelineHandle m_ClusterPipeline = 0;
		PipelineHandle m_ClusterCullLightPipeline = 0;
	};
}
