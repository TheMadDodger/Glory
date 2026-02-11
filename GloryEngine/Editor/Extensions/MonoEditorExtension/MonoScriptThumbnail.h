#pragma once
#include <ThumbnailGenerator.h>
#include <MonoScript.h>

namespace Glory::Editor
{
    class MonoScriptThumbnail : public ThumbnailGenerator<MonoScript>
    {
	public:
		MonoScriptThumbnail();
		virtual ~MonoScriptThumbnail();

		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pScriptThumbnail;
    };
}
