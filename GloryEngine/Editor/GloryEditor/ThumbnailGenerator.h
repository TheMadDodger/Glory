#pragma once
#include "ThumbnailManager.h"
#include "ResourceMeta.h"

namespace Glory::Editor
{
	class BaseThumbnailGenerator
	{
	public:
		GLORY_EDITOR_API BaseThumbnailGenerator();
		virtual GLORY_EDITOR_API ~BaseThumbnailGenerator();
		virtual GLORY_EDITOR_API TextureData* GetThumbnail(const ResourceMeta* pResourceType) = 0;
		virtual GLORY_EDITOR_API void OnFileDoubleClick(UUID uuid);

	protected:
		virtual GLORY_EDITOR_API const std::type_info& GetAssetType() = 0;
		virtual GLORY_EDITOR_API const std::string& GetExtension();

	private:
		friend class ThumbnailManager;
		static const std::string m_DefaultExtension;
	};

	template<class T>
	class ThumbnailGenerator : public BaseThumbnailGenerator
	{
	public:
		ThumbnailGenerator() {}
		virtual ~ThumbnailGenerator() {}
		const std::type_info& GetAssetType() override { return typeid(T); }
		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) = 0;
	};
}