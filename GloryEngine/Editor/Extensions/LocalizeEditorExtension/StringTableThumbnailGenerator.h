#pragma once
#include <ThumbnailGenerator.h>
#include <StringsOverrideTable.h>

namespace Glory::Editor
{
    class StringTableThumbnailGenerator : public ThumbnailGenerator<StringTable>
    {
	public:
		StringTableThumbnailGenerator();
		virtual ~StringTableThumbnailGenerator();

		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pTextDBThumbnail;
    };

	class StringsOverrideTableThumbnailGenerator : public ThumbnailGenerator<StringsOverrideTable>
	{
	public:
		StringsOverrideTableThumbnailGenerator();
		virtual ~StringsOverrideTableThumbnailGenerator();

		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pTextDBThumbnail;
	};
}
