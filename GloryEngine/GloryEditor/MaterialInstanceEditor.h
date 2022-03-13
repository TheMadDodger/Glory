#pragma once
#include "Editor.h"
#include <MaterialInstanceData.h>

namespace Glory::Editor
{
    class MaterialInstanceEditor : public EditorTemplate<MaterialInstanceEditor, MaterialInstanceData>
    {
    public:
        MaterialInstanceEditor();
        virtual ~MaterialInstanceEditor();

    private:
        virtual void OnGUI() override;
        virtual void Initialize() override;
    };
}
