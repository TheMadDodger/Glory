#include "NullRendererModule.h"

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(NullRendererModule);

	NullRendererModule::NullRendererModule()
	{
	}

	NullRendererModule::~NullRendererModule()
	{
	}

	void NullRendererModule::GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos)
	{
	}

	void NullRendererModule::OnCameraResize(CameraRef camera)
	{
	}

	void NullRendererModule::OnCameraPerspectiveChanged(CameraRef camera)
	{
	}

	MaterialData* NullRendererModule::GetInternalMaterial(std::string_view name) const
	{
		return nullptr;
	}

	void NullRendererModule::CollectReferences(std::vector<UUID>& references)
	{
	}

	UUID NullRendererModule::TextPipelineID() const
	{
		return 0;
	}

	void NullRendererModule::Initialize()
	{
		RendererModule::Initialize();
	}

	void NullRendererModule::OnPostInitialize()
	{
	}

	void NullRendererModule::Update()
	{
	}

	void NullRendererModule::Cleanup()
	{
	}

	void NullRendererModule::OnRenderEffects(CameraRef camera, RenderTexture* pRenderTexture)
	{
	}

	void NullRendererModule::OnDoCompositing(CameraRef camera, uint32_t width, uint32_t height, RenderTexture* pRenderTexture)
	{
	}

	void NullRendererModule::OnDisplayCopy(RenderTexture* pRenderTexture, uint32_t width, uint32_t height)
	{
	}

	void NullRendererModule::OnRenderSkybox(CameraRef camera, CubemapData* pCubemap)
	{
	}

	void NullRendererModule::OnStartCameraRender(CameraRef camera, const FrameData<LightData>& lights)
	{
	}

	void NullRendererModule::OnEndCameraRender(CameraRef camera, const FrameData<LightData>& lights)
	{
	}

	void NullRendererModule::LoadSettings(ModuleSettings& settings)
	{
		RendererModule::LoadSettings(settings);
	}
}
