#pragma once
#include "ThumbnailGenerator.h"

#include <GScene.h>

namespace Glory
{
	class ImageData;
}

namespace Glory::Editor
{
	class SceneThumbnailGenerator : public ThumbnailGenerator<GScene>
	{
	public:
		SceneThumbnailGenerator();
		virtual ~SceneThumbnailGenerator();

		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		ImageData* m_pSceneThumbnail;
	};
}
