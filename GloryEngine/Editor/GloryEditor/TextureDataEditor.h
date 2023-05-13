#pragma once
#include "Editor.h"
#include <MaterialData.h>

namespace Glory::Editor
{
	class TextureDataEditor : public EditorTemplate<TextureDataEditor, TextureData>
	{
	public:
		TextureDataEditor();
		virtual ~TextureDataEditor();
		virtual bool OnGUI() override;
	};
}
