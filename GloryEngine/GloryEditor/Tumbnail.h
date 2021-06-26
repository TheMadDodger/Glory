//#pragma once
//#include "GUIDComparer.h"
//#include "TextureData.h"
//#include <MetaData.h>
//
//namespace Spartan::Editor
//{
//	class BaseTumbnailGenerator;
//
//	class Tumbnail
//	{
//	public:
//		static TextureData* GetFolderTumbnail();
//		static TextureData* GetTumbnail(const Spartan::Serialization::MetaData& metaData);
//		static TextureData* GetTumbnail(const std::string extension);
//
//		template<class T>
//		static void AddGenerator()
//		{
//			BaseTumbnailGenerator* pGenerator = new T();
//			AddGenerator(pGenerator);
//		}
//
//		static void Destroy();
//
//	private:
//		static BaseTumbnailGenerator* GetGenerator(size_t hashCode);
//		static BaseTumbnailGenerator* GetGenerator(const std::string& extension);
//		static void AddGenerator(BaseTumbnailGenerator* pGenerator);
//
//	private:
//		static std::vector<BaseTumbnailGenerator*> m_pGenerators;
//		static std::map<GUID, TextureData*, Spartan::Serialization::GUIDComparer> m_pTumbnails;
//		static TextureData* m_pFolderTumbnail;
//
//	private:
//		Tumbnail();
//		virtual ~Tumbnail();
//	};
//}
