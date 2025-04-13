#pragma once
#include <TumbnailGenerator.h>
#include <TextDatabase.h>

namespace Glory::Editor
{
    class TextDatabaseTumbnailGenerator : public TumbnailGenerator<TextDatabase>
    {
	public:
		TextDatabaseTumbnailGenerator();
		virtual ~TextDatabaseTumbnailGenerator();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pTextDBTumbnail;
    };
}
