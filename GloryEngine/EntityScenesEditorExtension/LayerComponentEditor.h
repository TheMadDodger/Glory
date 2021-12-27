#pragma once
#include <Editor.h>
#include <Components.h>
#include "EntityComponentEditor.h"

namespace Glory::Editor
{
    class LayerComponentEditor : public EntityComponentEditor<LayerComponentEditor, LayerComponent>
    {
    public:
        LayerComponentEditor();
        virtual ~LayerComponentEditor();

        virtual void Initialize() override;

    private:
        virtual void OnGUI() override;
        virtual std::string Name() override;

    private:
        std::vector<std::string> m_LayerOptions;
        std::string m_LayerText;
    };
}
