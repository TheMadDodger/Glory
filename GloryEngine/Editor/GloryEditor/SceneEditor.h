#pragma once
#include "Editor.h"

#include <GScene.h>

namespace Glory::Editor
{
	class SceneEditor : public EditorTemplate<SceneEditor, GScene>
	{
	public:
		SceneEditor();
		virtual ~SceneEditor();
		virtual bool OnGUI() override;
	};
}
