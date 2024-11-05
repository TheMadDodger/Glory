#pragma once
#include "Editor.h"
#include "EditableResource.h"

#include <MaterialData.h>

namespace Glory::Editor
{
	class TextureDataEditor : public EditorTemplate<TextureDataEditor, YAMLResource<TextureData>>
	{
	public:
		TextureDataEditor();
		virtual ~TextureDataEditor();
		virtual bool OnGUI() override;
	};

	class StaticTextureDataEditor : public EditorTemplate<StaticTextureDataEditor, TextureData>
	{
	public:
		StaticTextureDataEditor();
		virtual ~StaticTextureDataEditor();
		virtual bool OnGUI() override;
	};
}
