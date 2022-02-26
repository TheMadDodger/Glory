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
		return (ImageData*)AssetManager::GetAssetImmediate(pResourceType->ID());
	}
}