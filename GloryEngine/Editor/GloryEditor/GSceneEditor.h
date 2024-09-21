#pragma once
#include "Editor.h"

#include <GScene.h>

namespace Glory::Editor
{
    class GSceneEditor : public EditorTemplate<GSceneEditor, GScene>
    {
    public:
        GSceneEditor() {}
        virtual ~GSceneEditor() = default;
        virtual bool OnGUI() override;
    };
}
