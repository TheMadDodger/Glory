#pragma once
#include "Editor.h"
#include <MaterialData.h>

namespace Glory::Editor
{
	class MaterialEditor : public EditorTemplate<MaterialEditor, MaterialData>
	{
	public:
		MaterialEditor();
		virtual ~MaterialEditor();
		virtual void OnGUI() override;
	};
}
