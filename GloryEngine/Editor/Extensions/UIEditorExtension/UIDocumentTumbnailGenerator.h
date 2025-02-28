#pragma once
#include <TumbnailGenerator.h>
#include <UIDocumentData.h>

namespace Glory::Editor
{
    class UIDocumentTumbnailGenerator : public TumbnailGenerator<UIDocumentData>
    {
	public:
		UIDocumentTumbnailGenerator();
		virtual ~UIDocumentTumbnailGenerator();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pUIDocTumbnail;
    };
}
