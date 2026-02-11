#pragma once
#include <ThumbnailGenerator.h>
#include <FSM.h>

namespace Glory::Editor
{
    class FSMThumbnailGenerator : public ThumbnailGenerator<FSMData>
    {
	public:
		FSMThumbnailGenerator();
		virtual ~FSMThumbnailGenerator();

		virtual TextureData* GetThumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pFSMThumbnail;
    };
}
