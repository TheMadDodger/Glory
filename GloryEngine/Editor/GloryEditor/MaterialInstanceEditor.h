#pragma once
#include "Editor.h"
#include "EditableResource.h"

#include <MaterialInstanceData.h>

namespace Glory::Editor
{
    class MaterialInstanceEditor : public EditorTemplate<MaterialInstanceEditor, YAMLResource<MaterialInstanceData>>
    {
    public:
        MaterialInstanceEditor();
        virtual ~MaterialInstanceEditor();

        void DrawErrorWindow(const char* error);

    private:
        virtual bool OnGUI() override;
        virtual void Initialize() override;
    };
}
