#include "EditorAssetDatabase.h"
#include "EditorAssets.h"
#include "ThumbnailManager.h"
#include "ThumbnailGenerator.h"
#include "EditorApplication.h"

#include <Engine.h>
#include <GraphicsDevice.h>

namespace Glory::Editor
{
	ThumbnailManager::ThumbnailManager(EditorApplication* pApp): m_pApplication(pApp) {}

	ThumbnailManager::~ThumbnailManager()
	{
		for (size_t i = 0; i < m_pGenerators.size(); i++)
		{
			delete m_pGenerators[i];
		}
		m_pGenerators.clear();
	}

	TextureHandle ThumbnailManager::GetThumbnail(UUID uuid)
	{
		GraphicsDevice* pDevice = EditorApplication::GetInstance()->GetEngine()->ActiveGraphicsDevice();

		auto it = m_pThumbnails.find(uuid);
		if (it != m_pThumbnails.end())
		{
			TextureData* pTextureData = m_pThumbnails.at(uuid);
			if (pDevice->CachedTextureExists(pTextureData))
				return pDevice->AcquireCachedTexture(pTextureData);
			return EditorAssets::GetTexture("file");
		}

		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(uuid, meta);

		BaseThumbnailGenerator* pGenerator = GetGenerator(meta.Hash());
		if (pGenerator == nullptr)
			return EditorAssets::GetTexture("file");

		TextureData* pImage = pGenerator->GetThumbnail(&meta);

		if (pImage == nullptr)
			return EditorAssets::GetTexture("file");

		m_pThumbnails.emplace(uuid, pImage);
		EditorAssets::EnqueueTextureCreation(pImage);
		return EditorAssets::GetTexture("file");
	}

	void ThumbnailManager::SetDirty(UUID uuid)
	{
		auto it = m_pThumbnails.find(uuid);
		if (it == m_pThumbnails.end()) return;
		m_pThumbnails.erase(uuid);
	}

	void ThumbnailManager::AddGenerator(BaseThumbnailGenerator* pGenerator)
	{
		m_pGenerators.push_back(pGenerator);
	}

	BaseThumbnailGenerator* ThumbnailManager::GetGenerator(uint32_t hashCode)
	{
		for (size_t i = 0; i < m_pGenerators.size(); i++)
		{
			const std::type_info& type = m_pGenerators[i]->GetAssetType();
			uint32_t hash = ResourceTypes::GetHash(type);
			if (hash != hashCode) continue;
			return m_pGenerators[i];
		}

		return nullptr;
	}
}
