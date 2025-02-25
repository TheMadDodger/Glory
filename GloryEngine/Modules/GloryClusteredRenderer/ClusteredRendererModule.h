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
	public:
		ClusteredRendererModule();
		virtual ~ClusteredRendererModule();

		virtual void GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos) override;
		virtual void OnCameraResize(CameraRef camera) override;
		virtual void OnCameraPerspectiveChanged(CameraRef camera) override;

		virtual void CollectReferences(std::vector<UUID>& references) override;

		GLORY_MODULE_VERSION_H(0,4,0);

	private:
		virtual void Cleanup() override;
		virtual void OnPostInitialize() override;

		virtual void OnRender(CameraRef camera, const RenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) override;
		virtual void OnRender(CameraRef camera, const TextRenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) override;
		virtual void OnRenderEffects(CameraRef camera, RenderTexture* pRenderTexture) override;
		virtual void OnDoCompositing(CameraRef camera, const FrameData<PointLight>& lights, uint32_t width, uint32_t height, RenderTexture* pRenderTexture) override;
		virtual void OnDisplayCopy(RenderTexture* pRenderTexture, uint32_t width, uint32_t height) override;

		virtual void OnStartCameraRender(CameraRef camera, const FrameData<PointLight>& lights) override;
		virtual void OnEndCameraRender(CameraRef camera, const FrameData<PointLight>& lights) override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		size_t GetGCD(size_t a, size_t b); // TODO: Move this to somewhere it can be used from anywhere and make it take templates

		void GenerateClusterSSBO(Buffer* pBuffer, CameraRef camera);
		void GenerateDomeSamplePointsSSBO(GPUResourceManager* pResourceManager, uint32_t size);

	private:
		// Compute shaders
		FileData* m_pClusterShaderData = nullptr;
		PipelineData* m_pClusterShaderPipelineData = nullptr;
		MaterialData* m_pClusterShaderMaterialData = nullptr;
		Material* m_pClusterShaderMaterial = nullptr;

		FileData* m_pMarkActiveClustersShaderData = nullptr;
		PipelineData* m_pMarkActiveClustersPipelineData = nullptr;
		MaterialData* m_pMarkActiveClustersMaterialData = nullptr;
		Material* m_pMarkActiveClustersMaterial = nullptr;

		FileData* m_pCompactClustersShaderData = nullptr;
		PipelineData* m_pCompactClustersPipelineData = nullptr;
		MaterialData* m_pCompactClustersMaterialData = nullptr;
		Material* m_pCompactClustersMaterial = nullptr;

		FileData* m_pClusterCullLightShaderData = nullptr;
		PipelineData* m_pClusterCullLightPipelineData = nullptr;
		MaterialData* m_pClusterCullLightMaterialData = nullptr;
		Material* m_pClusterCullLightMaterial = nullptr;

		// Data for clustering
		Buffer* m_pScreenToViewSSBO = nullptr;
		Buffer* m_pLightsSSBO = nullptr;
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

		std::map<UUID, std::unique_ptr<MeshData>> m_pTextMeshes;
	};
}
