#pragma once
#include <RendererModule.h>
#include <Material.h>
#include <MaterialData.h>
#include <FileData.h>
#include <glm/glm.hpp>

namespace Glory
{
	class PipelineData;
	class TextureAtlas;

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

	class ClusteredRendererModule : public RendererModule
	{
	private:
		struct PipelineBatchData
		{
			CPUBuffer<glm::mat4> m_Worlds;
			Buffer* m_pWorldsBuffer;

			std::vector<uint32_t> m_MaterialIndices;
			CPUBuffer<char> m_MaterialDatas;
			CPUBuffer<uint32_t> m_TextureBits;
			Buffer* m_pMaterialsBuffer;
			Buffer* m_pTextureBitsBuffer;
		};

	public:
		ClusteredRendererModule();
		virtual ~ClusteredRendererModule();

		virtual void GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos) override;
		virtual void OnCameraResize(CameraRef camera) override;
		virtual void OnCameraPerspectiveChanged(CameraRef camera) override;
		virtual MaterialData* GetInternalMaterial(std::string_view name) const override;

		virtual void CollectReferences(std::vector<UUID>& references) override;

		virtual UUID TextPipelineID() const override;

		GLORY_MODULE_VERSION_H(0,5,0);

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

	private:
		size_t GetGCD(size_t a, size_t b); // TODO: Move this to somewhere it can be used from anywhere and make it take templates

		void GenerateClusterSSBO(Buffer* pBuffer, CameraRef camera);
		void GenerateDomeSamplePointsSSBO(GPUResourceManager* pResourceManager, uint32_t size);

		void RenderBatches(const std::vector<PipelineBatch>& batches, const std::vector<PipelineBatchData>& batchDatas, size_t cameraIndex);

		void PrepareDataPass();
		void PrepareBatches(const std::vector<PipelineBatch>& batches, std::vector<PipelineBatchData>& batchDatas);
		void ShadowMapsPass(uint32_t cameraIndex);
		void RenderShadows(size_t lightIndex, const RenderFrame& frameData);

		void GenerateShadowLODDivisions(uint32_t maxLODs);
		void GenerateShadowMapLODResolutions();
		void ResizeShadowMapLODResolutions(uint32_t minSize, uint32_t maxSize);
		void ResizeShadowAtlas(uint32_t newSize);

		void StaticObjectsPass(uint32_t cameraIndex);
		void DynamicObjectsPass(uint32_t cameraIndex);
		void SkyboxPass(uint32_t cameraIndex);
		void DynamicLateObjectPass(uint32_t cameraIndex);
		void DeferredCompositePass(uint32_t cameraIndex);

	private:
		// Compute shaders
		MaterialData* m_pClusterShaderMaterialData = nullptr;
		MaterialData* m_pMarkActiveClustersMaterialData = nullptr;
		MaterialData* m_pCompactClustersMaterialData = nullptr;
		MaterialData* m_pClusterCullLightMaterialData = nullptr;

		// Data for clustering
		Buffer* m_pScreenToViewSSBO = nullptr;
		Buffer* m_pLightsSSBO = nullptr;
		Buffer* m_pLightCountSSBO = nullptr;
		Buffer* m_pLightSpaceTransformsSSBO = nullptr;
		Buffer* m_pLightDistancesSSBO = nullptr;
		Buffer* m_pSamplePointsDomeSSBO = nullptr;
		Buffer* m_pSSAOSettingsSSBO = nullptr;
		Texture* m_pSampleNoiseTexture = nullptr;

		static const size_t m_GridSizeX = 16;
		static const size_t m_GridSizeY = 9;
		static const size_t NUM_DEPTH_SLICES = 24;
		static const size_t NUM_CLUSTERS = m_GridSizeX * m_GridSizeY * NUM_DEPTH_SLICES;
		static const size_t MAX_LIGHTS_PER_TILE = 50;
		static const size_t MAX_KERNEL_SIZE = 1024;

		uint32_t m_SSAOKernelSize = 0;

		// Screen rendering
		MaterialData* m_pDeferredCompositeMaterial = nullptr;
		MaterialData* m_pDisplayCopyMaterial = nullptr;
		MaterialData* m_pSSRMaterial = nullptr;
		MaterialData* m_pSSAOMaterial = nullptr;
		MaterialData* m_pSSAOBlurMaterial = nullptr;

		Mesh* m_pQuadMesh;
		Buffer* m_pQuadMeshVertexBuffer;
		Buffer* m_pQuadMeshIndexBuffer;

		MaterialData* m_pTextMaterialData = nullptr;
		Material* m_pTextMaterial = nullptr;

		MaterialData* m_pSkyboxMaterialData = nullptr;
		MaterialData* m_pIrradianceMaterialData = nullptr;
		MaterialData* m_pShadowsMaterialData = nullptr;
		MaterialData* m_pShadowsTransparentMaterialData = nullptr;

		std::map<UUID, std::unique_ptr<MeshData>> m_pTextMeshes;

		SSAOSettings m_GlobalSSAOSetting;

		uint32_t m_MinShadowResolution;
		uint32_t m_MaxShadowResolution;
		uint32_t m_ShadowAtlasResolution;
		uint32_t m_MaxShadowLODs;
		static const uint32_t MAX_SHADOW_LODS = 24;
		std::vector<uint32_t> m_ShadowLODDivisions;
		std::vector<glm::uvec2> m_ShadowMapResolutions;

		GPUTextureAtlas* m_pShadowAtlas;

		std::vector<PipelineBatchData> m_StaticBatchData;
		std::vector<PipelineBatchData> m_DynamicBatchData;
		std::vector<PipelineBatchData> m_DynamicLateBatchData;

		CPUBuffer<PerCameraData> m_CameraDatas;
		CPUBuffer<PerCameraData> m_LightCameraDatas;
		Buffer* m_pCameraDatasBuffer;
		Buffer* m_pLightCameraDatasBuffer;
		Buffer* m_pRenderConstantsBuffer;
	};
}
