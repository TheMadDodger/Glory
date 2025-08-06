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

	class NullRendererModule : public RendererModule
	{
	public:
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
	};
}
