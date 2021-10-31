#include "TextureTumbnailGenerator.h"
#include "AssetManager.h"

namespace Glory::Editor
{
	TextureTumbnailGenerator::TextureTumbnailGenerator()
	{
	}

	TextureTumbnailGenerator::~TextureTumbnailGenerator()
	{
	}

	ImageData* TextureTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
	{
		return AssetManager::GetAsset<ImageData>(pResourceType->ID());
	}
}