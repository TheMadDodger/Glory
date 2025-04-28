#pragma once
#include <TumbnailGenerator.h>
#include <StringsOverrideTable.h>

namespace Glory::Editor
{
    class StringTableTumbnailGenerator : public TumbnailGenerator<StringTable>
    {
	public:
		StringTableTumbnailGenerator();
		virtual ~StringTableTumbnailGenerator();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pTextDBTumbnail;
    };

	class StringsOverrideTableTumbnailGenerator : public TumbnailGenerator<StringsOverrideTable>
	{
	public:
		StringsOverrideTableTumbnailGenerator();
		virtual ~StringsOverrideTableTumbnailGenerator();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pTextDBTumbnail;
	};
}
