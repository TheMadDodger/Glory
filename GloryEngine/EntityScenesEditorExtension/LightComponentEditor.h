#pragma once
#include <Editor.h>
#include <Components.h>
#include "EntityComponentEditor.h"

namespace Glory::Editor
{
    class LightComponentEditor : public EntityComponentEditor<LightComponentEditor, LightComponent>
    {
    public:
        LightComponentEditor();
        virtual ~LightComponentEditor();

    private:
        virtual void OnGUI() override;
        virtual std::string Name() override;
    };
}

