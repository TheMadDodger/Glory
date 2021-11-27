#pragma once
#include <Editor.h>
#include <Components.h>
#include "EntityComponentEditor.h"

namespace Glory::Editor
{
    class TransformComponentEditor : public EntityComponentEditor<TransformComponentEditor, Transform>
    {
    public:
        TransformComponentEditor();
        virtual ~TransformComponentEditor();

    private:
        virtual void OnGUI() override;
        virtual std::string Name() override;
    };
}
