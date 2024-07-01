#pragma once
#include <Editor.h>
#include <EditableResource.h>
#include <SoundMaterialData.h>

namespace Glory::Editor
{
    class SoundMaterialEditor : public EditorTemplate<SoundMaterialEditor, YAMLResource<SoundMaterialData>>
    {
	public:
		SoundMaterialEditor();
		virtual ~SoundMaterialEditor();
		virtual bool OnGUI() override;
    };
}
