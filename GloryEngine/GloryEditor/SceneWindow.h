#pragma once
#include "EditorWindow.h"
//#include <RenderTexture.h>

namespace Glory::Editor
{
    class SceneWindow : public EditorWindowTemplate<SceneWindow>
    {
    public:
        SceneWindow();
        virtual ~SceneWindow();

    private:
        virtual void OnPaint() override;
        virtual void OnGUI() override;

    private:
        //RenderTexture* m_pRenderTexture;
    };
}