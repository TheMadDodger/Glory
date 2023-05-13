#include <Engine.h>
#include "EditorAssetDatabase.h"
#include "EditorAssets.h"
#include "EditorAssets.h"
#include "Tumbnail.h"
#include "TumbnailGenerator.h"

namespace Glory::Editor
{
	std::vector<BaseTumbnailGenerator*> Tumbnail::m_pGenerators;
	std::map<UUID, TextureData*> Tumbnail::m_pTumbnails;

	Texture* Tumbnail::GetTumbnail(UUID uuid)
	{
		GPUResourceManager* pResourceManager = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager();

		auto it = m_pTumbnails.find(uuid);
		if (it != m_pTumbnails.end())
		{
			TextureData* pTextureData = m_pTumbnails.at(uuid);
			if (pResourceManager->ResourceExists(pTextureData))
				return pResourceManager->CreateTexture(pTextureData);
			return EditorAssets::GetTexture("file");
		}

		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(uuid, meta);

		BaseTumbnailGenerator* pGenerator = GetGenerator(meta.Hash());
		if (pGenerator == nullptr)
			return EditorAssets::GetTexture("file");

		TextureData* pImage = pGenerator->GetTumbnail(&meta);

		if (pImage == nullptr)
			return EditorAssets::GetTexture("file");

		m_pTumbnails.emplace(uuid, pImage);
		EditorAssets::EnqueueTextureCreation(pImage);
		return EditorAssets::GetTexture("file");
	}

	void Tumbnail::SetDirty(UUID uuid)
	{
		auto it = m_pTumbnails.find(uuid);
		if (it == m_pTumbnails.end()) return;
		m_pTumbnails.erase(uuid);
	}

	void Tumbnail::AddGenerator(BaseTumbnailGenerator* pGenerator)
	{
		m_pGenerators.push_back(pGenerator);
	}

	GLORY_EDITOR_API void Tumbnail::Destroy()
	{
		for (size_t i = 0; i < m_pGenerators.size(); i++)
		{
			delete m_pGenerators[i];
		}
		m_pGenerators.clear();
	}

	GLORY_EDITOR_API BaseTumbnailGenerator* Tumbnail::GetGenerator(uint32_t hashCode)
	{
		for (size_t i = 0; i < m_pGenerators.size(); i++)
		{
			const std::type_info& type = m_pGenerators[i]->GetAssetType();
			uint32_t hash = ResourceType::GetHash(type);
			if (hash != hashCode) continue;
			return m_pGenerators[i];
		}

		return nullptr;
	}

	Tumbnail::Tumbnail() {}
	Tumbnail::~Tumbnail() {}
}
