#pragma once
#include <TumbnailGenerator.h>
#include <MonoScript.h>

namespace Glory::Editor
{
    class MonoScriptTumbnail : public TumbnailGenerator<MonoScript>
    {
	public:
		MonoScriptTumbnail();
		virtual ~MonoScriptTumbnail();

		virtual TextureData* GetTumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		TextureData* m_pScriptTumbnail;
    };
}
