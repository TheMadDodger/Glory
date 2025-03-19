#include "HDRTumbnailGenerator.h"
#include "EditorApplication.h"
#include "AssetLocation.h"
#include "EditorAssetDatabase.h"

#include <Engine.h>
#include <AssetManager.h>

namespace Glory::Editor
{
	HDRTumbnailGenerator::HDRTumbnailGenerator()
	{
	}

	HDRTumbnailGenerator::~HDRTumbnailGenerator()
	{
	}

	TextureData* HDRTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
	{
		UUID id = pResourceType->ID();
		if (std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), id) != m_AlreadyRequestedTumbnails.end()) return nullptr;

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

		m_AlreadyRequestedTumbnails.push_back(id);

		EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAsset(id, [&](Resource* pResource)
		{
			UUID uuid = pResource->GetUUID();
			auto it = std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), uuid);
			m_pLoadedImages[uuid] = (HDRImageData*)pResource;
			if (it == m_AlreadyRequestedTumbnails.end()) return;
			m_AlreadyRequestedTumbnails.erase(it);
		});
		return nullptr;
	}
}
