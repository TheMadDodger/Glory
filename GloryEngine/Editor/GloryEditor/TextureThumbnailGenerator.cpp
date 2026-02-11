#include "TextureThumbnailGenerator.h"
#include "AssetManager.h"
#include "EditorApplication.h"

namespace Glory::Editor
{
	TextureThumbnailGenerator::TextureThumbnailGenerator()
	{
	}

	TextureThumbnailGenerator::~TextureThumbnailGenerator()
	{
	}

	TextureData* TextureThumbnailGenerator::GetThumbnail(const ResourceMeta* pResourceType)
	{
		UUID id = pResourceType->ID();
		if (std::find(m_AlreadyRequestedThumbnails.begin(), m_AlreadyRequestedThumbnails.end(), id) != m_AlreadyRequestedThumbnails.end()) return nullptr;

		if (m_pLoadedTextures.find(id) != m_pLoadedTextures.end())
		{
			TextureData* pImage = m_pLoadedTextures.at(id);
			m_pLoadedTextures.erase(id);
			return pImage;
		}

		m_AlreadyRequestedThumbnails.push_back(id);

		EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAsset(id, [&](Resource* pResource)
		{
			if (!pResource) return;
			UUID uuid = pResource->GetUUID();
			auto it = std::find(m_AlreadyRequestedThumbnails.begin(), m_AlreadyRequestedThumbnails.end(), uuid);
			m_pLoadedTextures[uuid] = (TextureData*)pResource;
			if (it == m_AlreadyRequestedThumbnails.end()) return;
			m_AlreadyRequestedThumbnails.erase(it);
		});
		return nullptr;
	}
}