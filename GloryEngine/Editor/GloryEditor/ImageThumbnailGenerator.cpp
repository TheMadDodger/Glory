#include "ImageThumbnailGenerator.h"
#include "AssetManager.h"
#include "EditorApplication.h"
#include "EditorAssets.h"
#include "EditorAssetDatabase.h"

#include <TextureData.h>

namespace Glory::Editor
{
	ImageThumbnailGenerator::ImageThumbnailGenerator()
	{
	}

	ImageThumbnailGenerator::~ImageThumbnailGenerator()
	{
	}

	TextureData* ImageThumbnailGenerator::GetThumbnail(const ResourceMeta* pResourceType)
	{
		UUID id = pResourceType->ID();
		if (std::find(m_AlreadyRequestedThumbnails.begin(), m_AlreadyRequestedThumbnails.end(), id) != m_AlreadyRequestedThumbnails.end()) return nullptr;

		if (m_pLoadedImages.find(id) != m_pLoadedImages.end())
		{
			ImageData* pImage = m_pLoadedImages[id];
			m_pLoadedImages.erase(id);
			AssetLocation location;
			if (!EditorAssetDatabase::GetAssetLocation(id, location)) return nullptr;
			std::filesystem::path subPath = location.SubresourcePath;
			subPath.append("Default");
			const UUID defaultTextureID = EditorAssetDatabase::FindAssetUUID(location.Path, subPath);
			if (!defaultTextureID) return nullptr;
			Resource* pResource = EditorApplication::GetInstance()->GetEngine()->GetAssetManager().FindResource(defaultTextureID);
			if (!pResource) return nullptr;
			return static_cast<TextureData*>(pResource);
		}

		m_AlreadyRequestedThumbnails.push_back(id);

		EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAsset(id, [&](Resource* pResource)
		{
			UUID uuid = pResource->GetUUID();
			auto it = std::find(m_AlreadyRequestedThumbnails.begin(), m_AlreadyRequestedThumbnails.end(), uuid);
			m_pLoadedImages[uuid] = (ImageData*)pResource;
			if (it == m_AlreadyRequestedThumbnails.end()) return;
			m_AlreadyRequestedThumbnails.erase(it);
		});
		return nullptr;
	}
}