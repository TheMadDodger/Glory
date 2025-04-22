#pragma once
#include <TumbnailGenerator.h>
#include <StringTable.h>

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
}
