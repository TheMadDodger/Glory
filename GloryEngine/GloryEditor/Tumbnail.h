//#pragma once
//#include "ImageData.h"
//
//namespace Glory::Editor
//{
//	class BaseTumbnailGenerator;
//
//	class Tumbnail
//	{
//	public:
//		static ImageData* GetFolderTumbnail();
//		static ImageData* GetTumbnail();
//		static ImageData* GetTumbnail();
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
//		static std::map<UUID, ImageData*> m_pTumbnails;
//		static ImageData* m_pFolderTumbnail;
//
//	private:
//		Tumbnail();
//		virtual ~Tumbnail();
//	};
//}
