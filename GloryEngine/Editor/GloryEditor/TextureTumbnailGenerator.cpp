#include "TextureTumbnailGenerator.h"
#include "AssetManager.h"
#include "EditorApplication.h"

namespace Glory::Editor
{
	TextureTumbnailGenerator::TextureTumbnailGenerator()
	{
	}

	TextureTumbnailGenerator::~TextureTumbnailGenerator()
	{
	}

	TextureData* TextureTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
	{
		UUID id = pResourceType->ID();
		if (std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), id) != m_AlreadyRequestedTumbnails.end()) return nullptr;

		if (m_pLoadedTextures.find(id) != m_pLoadedTextures.end())
		{
			TextureData* pImage = m_pLoadedTextures.at(id);
			m_pLoadedTextures.erase(id);
			return pImage;
		}

		m_AlreadyRequestedTumbnails.push_back(id);

		EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAsset(id, [&](Resource* pResource)
		{
			if (!pResource) return;
			UUID uuid = pResource->GetUUID();
			auto it = std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), uuid);
			m_pLoadedTextures[uuid] = (TextureData*)pResource;
			if (it == m_AlreadyRequestedTumbnails.end()) return;
			m_AlreadyRequestedTumbnails.erase(it);
		});
		return nullptr;
	}
}