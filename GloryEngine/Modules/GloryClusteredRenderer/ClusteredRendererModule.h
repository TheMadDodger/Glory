#pragma once
#include <RendererModule.h>
#include <Material.h>
#include <MaterialData.h>
#include <FileData.h>
#include <glm/glm.hpp>

namespace Glory
{
	class PipelineData;

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
	};

	struct LightGrid
	{
		uint32_t Offset;
		uint32_t Count;
	};

	class ClusteredRendererModule : public RendererModule
	{
	public:
		ClusteredRendererModule();
		virtual ~ClusteredRendererModule();

		virtual void GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos) override;
		virtual void OnCameraResize(CameraRef camera) override;
		virtual void OnCameraPerspectiveChanged(CameraRef camera) override;

		GLORY_MODULE_VERSION_H(0,2,0);

	private:
		virtual void Cleanup() override;
		virtual void OnPostInitialize() override;

		virtual void OnThreadedInitialize() override;
		virtual void OnThreadedCleanup() override;

		virtual void OnRender(CameraRef camera, const RenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) override;
		virtual void OnRenderEffects(CameraRef camera, RenderTexture* pRenderTexture) override;
		virtual void OnDoScreenRender(CameraRef camera, const FrameData<PointLight>& lights, uint32_t width, uint32_t height, RenderTexture* pRenderTexture) override;

		virtual void OnStartCameraRender(CameraRef camera, const FrameData<PointLight>& lights) override;
		virtual void OnEndCameraRender(CameraRef camera, const FrameData<PointLight>& lights) override;

	private:
		size_t GetGCD(size_t a, size_t b); // TODO: Move this to somewhere it can be used from anywhere and make it take templates

		void GenerateClusterSSBO(Buffer* pBuffer, CameraRef camera);
		void GenerateDomeSamplePointsSSBO(GPUResourceManager* pResourceManager);

	private:
		// Compute shaders
		FileData* m_pClusterShaderData;
		PipelineData* m_pClusterShaderPipelineData;
		MaterialData* m_pClusterShaderMaterialData;
		Material* m_pClusterShaderMaterial;

		FileData* m_pMarkActiveClustersShaderData;
		PipelineData* m_pMarkActiveClustersPipelineData;
		MaterialData* m_pMarkActiveClustersMaterialData;
		Material* m_pMarkActiveClustersMaterial;

		FileData* m_pCompactClustersShaderData;
		PipelineData* m_pCompactClustersPipelineData;
		MaterialData* m_pCompactClustersMaterialData;
		Material* m_pCompactClustersMaterial;

		FileData* m_pClusterCullLightShaderData;
		PipelineData* m_pClusterCullLightPipelineData;
		MaterialData* m_pClusterCullLightMaterialData;
		Material* m_pClusterCullLightMaterial;

		FileData* m_pScreenVertShader;
		FileData* m_pScreenFragShader;
		FileData* m_pSSRFragShader;
		FileData* m_pSSAOFragShader;
		FileData* m_pSSAOBlurFragShader;

		// Data for clustering
		Buffer* m_pScreenToViewSSBO;
		Buffer* m_pLightsSSBO;
		Buffer* m_pSamplePointsDomeSSBO;
		Texture* m_pSampleNoiseTexture;

		static const size_t m_GridSizeX = 16;
		static const size_t m_GridSizeY = 9;
		static const size_t NUM_DEPTH_SLICES = 24;
		static const size_t NUM_CLUSTERS = m_GridSizeX * m_GridSizeY * NUM_DEPTH_SLICES;
		static const size_t MAX_LIGHTS_PER_TILE = 50;

		static const size_t NUM_SAMPLE_POINTS = 64;

		// Screen rendering
		PipelineData* m_pScreenPipeline;
		PipelineData* m_pSSRPipeline;
		PipelineData* m_pSSAOPipeline;
		PipelineData* m_pSSAOBlurPipeline;
		MaterialData* m_pScreenMaterial;
		MaterialData* m_pSSRMaterial;
		MaterialData* m_pSSAOMaterial;
		MaterialData* m_pSSAOBlurMaterial;
	};
}
