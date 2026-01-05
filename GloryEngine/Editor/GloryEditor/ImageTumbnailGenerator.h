#pragma once
#include "TumbnailGenerator.h"

#include <ImageData.h>

namespace Glory::Editor
{
	class ImageTumbnailGenerator : public TumbnailGenerator<ImageData>
	{
	public:
		ImageTumbnailGenerator();
		virtual ~ImageTumbnailGenerator();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;

	private:
		std::vector<UUID> m_AlreadyRequestedTumbnails;
		std::map<UUID, ImageData*> m_pLoadedImages;
	};
}