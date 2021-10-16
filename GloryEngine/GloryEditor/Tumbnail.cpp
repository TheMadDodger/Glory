//#include "Tumbnail.h"
//#include "TumbnailGenerator.h"
//#include <AssetManager.h>
//
//namespace Spartan::Editor
//{
//	std::vector<BaseTumbnailGenerator*> Tumbnail::m_pGenerators;
//	std::map<GUID, TextureData*, Spartan::Serialization::GUIDComparer> Tumbnail::m_pTumbnails;
//	TextureData* Tumbnail::m_pFolderTumbnail = nullptr;
//
//	TextureData* Tumbnail::GetFolderTumbnail()
//	{
//		if (m_pFolderTumbnail) return m_pFolderTumbnail;
//		m_pFolderTumbnail = ContentManager::GetInstance()->Load<TextureData>("./../EditorAssets/folder.png");
//		return m_pFolderTumbnail;
//	}
//
//	TextureData* Tumbnail::GetTumbnail(const Spartan::Serialization::MetaData& metaData)
//	{
//		auto it = m_pTumbnails.find(metaData.m_GUID);
//		if (it != m_pTumbnails.end()) return m_pTumbnails[metaData.m_GUID];
//
//		BaseTumbnailGenerator* pGenerator = GetGenerator(metaData.m_HashCode);
//		if (pGenerator == nullptr) return nullptr;
//		TextureData* pTexture = pGenerator->GetTumbnail(metaData);
//		m_pTumbnails[metaData.m_GUID] = pTexture;
//		return pTexture;
//	}
//
//	TextureData* Tumbnail::GetTumbnail(const std::string extension)
//	{
//		BaseTumbnailGenerator* pGenerator = GetGenerator(extension);
//		if (pGenerator == nullptr) return nullptr;
//		TextureData* pTexture = pGenerator->GetTumbnail();
//		return pTexture;
//	}
//
//	void Tumbnail::AddGenerator(BaseTumbnailGenerator* pGenerator)
//	{
//		m_pGenerators.push_back(pGenerator);
//	}
//
//	void Tumbnail::Destroy()
//	{
//		for (size_t i = 0; i < m_pGenerators.size(); i++)
//		{
//			delete m_pGenerators[i];
//		}
//		m_pGenerators.clear();
//	}
//
//	BaseTumbnailGenerator* Tumbnail::GetGenerator(size_t hashCode)
//	{
//		for (size_t i = 0; i < m_pGenerators.size(); i++)
//		{
//			const std::type_info& type = m_pGenerators[i]->GetAssetType();
//			size_t hash = SEObject::GetClassHash(type);
//			if (hash != hashCode) continue;
//			return m_pGenerators[i];
//		}
//
//		return nullptr;
//	}
//
//	BaseTumbnailGenerator* Tumbnail::GetGenerator(const std::string& extension)
//	{
//		for (size_t i = 0; i < m_pGenerators.size(); i++)
//		{
//			const std::string& otherExt = m_pGenerators[i]->GetExtension();
//			if (otherExt != extension) continue;
//			return m_pGenerators[i];
//		}
//
//		return nullptr;
//	}
//
//	Tumbnail::Tumbnail() {}
//	Tumbnail::~Tumbnail() {}
//}
