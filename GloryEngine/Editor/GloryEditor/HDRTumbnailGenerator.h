#pragma once
#include "TumbnailGenerator.h"

#include <HDRImageData.h>

namespace Glory::Editor
{
    class HDRTumbnailGenerator : public TumbnailGenerator<HDRImageData>
	{
	public:
		HDRTumbnailGenerator();
		virtual ~HDRTumbnailGenerator();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;

	private:
		std::vector<UUID> m_AlreadyRequestedTumbnails;
		std::map<UUID, HDRImageData*> m_pLoadedImages;
		std::map<UUID, TextureData*> m_pTumbnails;
	};
}
