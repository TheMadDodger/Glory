#pragma once
#include "Tumbnail.h"
#include "ResourceMeta.h"

namespace Glory::Editor
{
	class BaseTumbnailGenerator
	{
	public:
		BaseTumbnailGenerator();
		virtual ~BaseTumbnailGenerator();

		virtual ImageData* GetTumbnail(const ResourceMeta* pResourceType) = 0;

		virtual void OnFileDoubleClick(UUID uuid);

	protected:
		virtual const std::type_info& GetAssetType() = 0;
		virtual const std::string& GetExtension();

	private:
		friend class Tumbnail;
	};

	template<class T>
	class TumbnailGenerator : public BaseTumbnailGenerator
	{
	public:
		TumbnailGenerator() {}
		virtual ~TumbnailGenerator() {}
		const std::type_info& GetAssetType() override { return typeid(T); }
		virtual ImageData* GetTumbnail(const ResourceMeta* pResourceType) = 0;
	};
}