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

		virtual ImageData* GetTumbnail(const ResourceMeta* pResourceType) override;
		virtual void OnFileDoubleClick(UUID uuid) override;

	private:
		ImageData* m_pScriptTumbnail;
    };
}
