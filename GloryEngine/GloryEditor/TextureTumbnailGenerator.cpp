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
		UUID id = pResourceType->ID();
		if (std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), id) != m_AlreadyRequestedTumbnails.end()) return nullptr;

		if (m_pLoadedImages.find(id) != m_pLoadedImages.end())
		{
			ImageData* pImage = m_pLoadedImages[id];
			m_pLoadedImages.erase(id);
			return pImage;
		}

		AssetManager::GetAsset(id, [&](Resource* pResource)
		{
			UUID uuid = pResource->GetUUID();
			auto it = std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), uuid);
			m_AlreadyRequestedTumbnails.erase(it);
			m_pLoadedImages[uuid] = (ImageData*)pResource;
		});

		m_AlreadyRequestedTumbnails.push_back(id);
		return nullptr;
	}
}