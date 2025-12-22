#pragma once
#include <RendererModule.h>
#include <FileData.h>
#include <glm/glm.hpp>

namespace Glory
{
	class PipelineData;
	class TextureAtlas;

	class NullRendererModule : public RendererModule
	{
	public:
		NullRendererModule();
		virtual ~NullRendererModule();

		virtual void OnCameraResize(CameraRef) override;
		virtual void OnCameraPerspectiveChanged(CameraRef) override;
		virtual MaterialData* GetInternalMaterial(std::string_view) const override;

		virtual void CollectReferences(std::vector<UUID>&) override;

		virtual UUID TextPipelineID() const override;
		virtual void PresentFrame() override;

		virtual uint32_t GetNumFramesInFlight() const override { return 0; };
		virtual uint32_t GetCurrentFrameInFlight() const override { return 0; };

		GLORY_MODULE_VERSION_H(1, 0, 0);

	private:
		virtual void Cleanup() override;
		virtual void Initialize() override;
		virtual void OnPostInitialize() override;
		virtual void Update() override;
		virtual void Draw() override;

		virtual size_t DefaultAttachmenmtIndex() const override;
		virtual size_t CameraAttachmentPreviewCount() const override;
		virtual std::string_view CameraAttachmentPreviewName(size_t) const override;
		virtual TextureHandle CameraAttachmentPreview(CameraRef, size_t) const override;
		virtual TextureHandle FinalColor() const override;
		virtual void VisualizeAttachment(CameraRef, size_t) override;

		virtual size_t DebugOverlayCount() const override;
		virtual std::string_view DebugOverlayName(size_t) const override;
		virtual void SetDebugOverlayEnabled(CameraRef, size_t, bool) override;
		virtual bool DebugOverlayEnabled(CameraRef, size_t) const override;

		virtual void LoadSettings(ModuleSettings& settings) override;
	};
}
