#pragma once
#include "TumbnailGenerator.h"

#include <TextureData.h>

namespace Glory::Editor
{
	class TextureTumbnailGenerator : public TumbnailGenerator<TextureData>
	{
	public:
		TextureTumbnailGenerator();
		virtual ~TextureTumbnailGenerator();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;

	private:
		std::vector<UUID> m_AlreadyRequestedTumbnails;
		std::map<UUID, TextureData*> m_pLoadedTextures;
	};
}