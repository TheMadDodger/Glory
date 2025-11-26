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

	void NullRendererModule::OnCameraResize(CameraRef)
	{
	}

	void NullRendererModule::OnCameraPerspectiveChanged(CameraRef)
	{
	}

	MaterialData* NullRendererModule::GetInternalMaterial(std::string_view) const
	{
		return nullptr;
	}

	void NullRendererModule::CollectReferences(std::vector<UUID>&)
	{
	}

	UUID NullRendererModule::TextPipelineID() const
	{
		return 0;
	}

	void NullRendererModule::PresentFrame()
	{
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

	size_t NullRendererModule::DefaultAttachmenmtIndex() const
	{
		return 0;
	}

	size_t NullRendererModule::CameraAttachmentPreviewCount() const
	{
		return 0;
	}

	std::string_view NullRendererModule::CameraAttachmentPreviewName(size_t) const
	{
		return "";
	}

	TextureHandle NullRendererModule::CameraAttachmentPreview(CameraRef, size_t) const
	{
		return NULL;
	}

	TextureHandle NullRendererModule::FinalColor() const
	{
		return NULL;
	}

	void NullRendererModule::VisualizeAttachment(CameraRef, size_t)
	{
	}

	size_t NullRendererModule::DebugOverlayCount() const
	{
		return 0;
	}

	std::string_view NullRendererModule::DebugOverlayName(size_t) const
	{
		return "";
	}

	TextureHandle NullRendererModule::DebugOverlay(size_t) const
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
