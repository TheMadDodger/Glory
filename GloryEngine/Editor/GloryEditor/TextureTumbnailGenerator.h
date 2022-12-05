#pragma once
#include "TumbnailGenerator.h"

namespace Glory::Editor
{
	class TextureTumbnailGenerator : public TumbnailGenerator<ImageData>
	{
	public:
		TextureTumbnailGenerator();
		virtual ~TextureTumbnailGenerator();

		virtual ImageData* GetTumbnail(const ResourceMeta* pResourceType) override;

	private:
		std::vector<UUID> m_AlreadyRequestedTumbnails;
		std::map<UUID, ImageData*> m_pLoadedImages;
	};
}