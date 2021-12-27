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

    private:
        virtual void OnGUI() override;
        virtual std::string Name() override;
    };
}
