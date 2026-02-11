#pragma once
#include "ThumbnailGenerator.h"

#include <ImageData.h>

namespace Glory::Editor
{
	class ImageThumbnailGenerator : public ThumbnailGenerator<ImageData>
	{
	public:
		ImageThumbnailGenerator();
		virtual ~ImageThumbnailGenerator();

		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) override;

	private:
		std::vector<UUID> m_AlreadyRequestedThumbnails;
		std::map<UUID, ImageData*> m_pLoadedImages;
	};
}