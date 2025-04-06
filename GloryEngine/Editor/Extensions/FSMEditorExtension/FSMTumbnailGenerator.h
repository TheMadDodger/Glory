#pragma once
#include <TumbnailGenerator.h>
#include <FSM.h>

namespace Glory::Editor
{
    class FSMTumbnailGenerator : public TumbnailGenerator<FSMData>
    {
	public:
		FSMTumbnailGenerator();
		virtual ~FSMTumbnailGenerator();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pFSMTumbnail;
    };
}
