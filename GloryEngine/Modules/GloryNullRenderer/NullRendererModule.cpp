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

	void NullRendererModule::Draw()
	{
	}

	size_t NullRendererModule::CameraAttachmentPreviewCount() const
	{
		return 0;
	}

	std::string_view NullRendererModule::CameraAttachmentPreviewName(size_t index) const
	{
		return "";
	}

	TextureHandle NullRendererModule::CameraAttachmentPreview(CameraRef camera, size_t index) const
	{
		return NULL;
	}

	void NullRendererModule::Cleanup()
	{
	}

	void NullRendererModule::LoadSettings(ModuleSettings& settings)
	{
		RendererModule::LoadSettings(settings);
	}
}
