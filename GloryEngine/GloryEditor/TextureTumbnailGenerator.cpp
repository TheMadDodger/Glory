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

		m_AlreadyRequestedTumbnails.push_back(id);

		AssetManager::GetAsset(id, [&](Resource* pResource)
		{
			UUID uuid = pResource->GetUUID();
			auto it = std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), uuid);
			m_pLoadedImages[uuid] = (ImageData*)pResource;
			if (it == m_AlreadyRequestedTumbnails.end()) return;
			m_AlreadyRequestedTumbnails.erase(it);
		});
		return nullptr;
	}
}