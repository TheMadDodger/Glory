#pragma once
#include "Tumbnail.h"
#include "ResourceMeta.h"

namespace Glory::Editor
{
	class BaseTumbnailGenerator
	{
	public:
		GLORY_EDITOR_API BaseTumbnailGenerator();
		virtual GLORY_EDITOR_API ~BaseTumbnailGenerator();

		virtual ImageData* GetTumbnail(const ResourceMeta* pResourceType) = 0;

		virtual GLORY_EDITOR_API void OnFileDoubleClick(UUID uuid);

	protected:
		virtual const std::type_info& GetAssetType() = 0;
		virtual GLORY_EDITOR_API const std::string& GetExtension();

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