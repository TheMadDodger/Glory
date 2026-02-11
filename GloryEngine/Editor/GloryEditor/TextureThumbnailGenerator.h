#pragma once
#include "ThumbnailGenerator.h"

#include <TextureData.h>

namespace Glory::Editor
{
	class TextureThumbnailGenerator : public ThumbnailGenerator<TextureData>
	{
	public:
		TextureThumbnailGenerator();
		virtual ~TextureThumbnailGenerator();

		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) override;

	private:
		std::vector<UUID> m_AlreadyRequestedThumbnails;
		std::map<UUID, TextureData*> m_pLoadedTextures;
	};
}