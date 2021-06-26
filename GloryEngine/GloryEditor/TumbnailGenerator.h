//#pragma once
//#include "Tumbnail.h"
//#include "MetaData.h"
//
//namespace Spartan::Editor
//{
//	class BaseTumbnailGenerator
//	{
//	public:
//		BaseTumbnailGenerator();
//		virtual ~BaseTumbnailGenerator();
//
//		virtual TextureData* GetTumbnail(const Spartan::Serialization::MetaData& metaData) = 0;
//		virtual TextureData* GetTumbnail();
//
//	protected:
//		virtual const std::type_info& GetAssetType() = 0;
//		virtual const std::string& GetExtension();
//
//	private:
//		friend class Tumbnail;
//	};
//
//	class ExtensionTumbnail : public BaseTumbnailGenerator
//	{
//	public:
//		ExtensionTumbnail(const std::string& extension);
//		virtual ~ExtensionTumbnail();
//
//	protected:
//		virtual const std::string& GetExtension() const;
//
//	private:
//		const std::string m_Extension;
//	};
//
//	template<class T>
//	class TumbnailGenerator : public BaseTumbnailGenerator
//	{
//	public:
//		TumbnailGenerator() {}
//		virtual ~TumbnailGenerator() {}
//		const std::type_info& GetAssetType() override { return typeid(T); }
//		virtual TextureData* GetTumbnail(const Spartan::Serialization::MetaData& metaData) = 0;
//	};
//}