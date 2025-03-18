#pragma once
#include "EntityComponentEditor.h"
#include <Components.h>

namespace Glory::Editor
{
    class LightComponentEditor : public EntityComponentEditor<LightComponentEditor, LightComponent>
    {
    public:
        LightComponentEditor();
        virtual ~LightComponentEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;
    };
}