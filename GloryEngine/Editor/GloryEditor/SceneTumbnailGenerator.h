#pragma once
#include "TumbnailGenerator.h"
#include <GScene.h>

namespace Glory::Editor
{
	class SceneTumbnailGenerator : public TumbnailGenerator<GScene>
	{
	public:
		SceneTumbnailGenerator();
		virtual ~SceneTumbnailGenerator();

		virtual ImageData* GetTumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		ImageData* m_pSceneTumbnail;
	};
}
