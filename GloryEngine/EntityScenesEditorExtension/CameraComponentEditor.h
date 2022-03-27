#pragma once
#include <Editor.h>
#include <Components.h>
#include "EntityComponentEditor.h"

namespace Glory::Editor
{
    class CameraComponentEditor : public EntityComponentEditor<CameraComponentEditor, CameraComponent>
    {
    public:
        CameraComponentEditor();
        virtual ~CameraComponentEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;

    private:
        std::vector<std::string> m_LayerOptions;
        std::string m_LayerText;
    };
}
