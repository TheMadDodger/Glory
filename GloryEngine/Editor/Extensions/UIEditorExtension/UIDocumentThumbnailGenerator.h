#pragma once
#include <ThumbnailGenerator.h>
#include <UIDocumentData.h>

namespace Glory::Editor
{
    class UIDocumentThumbnailGenerator : public ThumbnailGenerator<UIDocumentData>
    {
	public:
		UIDocumentThumbnailGenerator();
		virtual ~UIDocumentThumbnailGenerator();

		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pUIDocThumbnail;
    };
}
