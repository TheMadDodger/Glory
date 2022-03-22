#pragma once
#include "EditorRenderImpl.h"
#include "EditorPlatform.h"
#include "imgui_impl_opengl3.h"

namespace Glory::Editor
{
    class EditorOpenGLRenderImpl : public EditorRenderImpl
    {
    public:
        EditorOpenGLRenderImpl();
        virtual ~EditorOpenGLRenderImpl();

    private:
        virtual void Setup() override;
        virtual void SetupBackend() override;
        virtual void UploadImGUIFonts() override;
        virtual void Shutdown() override;
        virtual void Cleanup() override;
        virtual void BeforeRender() override;
        virtual void NewFrame() override;
        virtual void Clear(const ImVec4& clearColor) override;
        virtual void FrameRender(ImDrawData* pDrawData) override;
        virtual void FramePresent() override;

        virtual void* GetTextureID(Texture* pTexture) override;

        void LogGLError(const unsigned int& err, bool bIncludeTimeStamp);

    private:
        const int MINIMAGECOUNT = 2;
    };
}
