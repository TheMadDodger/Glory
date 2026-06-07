#pragma once
#include "EditorRenderImpl.h"
#include "EditorPlatform.h"
#include "imgui_impl_opengl3.h"

#include <GloryOGL.h>
#include <EditorCreateInfo.h>

#ifdef GLORY_EDITOROGL_EXPORTS
// BUILD LIB
#define GLORY_EDITOROGL_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_EDITOROGL_API __declspec(dllimport)
#endif

namespace Glory::Editor
{
    extern "C" GLORY_EDITOROGL_API void LoadBackend(EditorCreateInfo& editorCreateInfo);

    class EditorOpenGLRenderImpl : public EditorRenderImpl
    {
    public:
        EditorOpenGLRenderImpl();
        virtual ~EditorOpenGLRenderImpl();

    private:
        virtual std::string ShadingLanguage() override;
        virtual bool PushConstantsSupported() override { return false; }
        virtual void CompileShaderForEditor(const EditorShaderData& editorShader, std::vector<char>& out) override;
        virtual const std::vector<std::string>& GetDeviceUniqueDefines() const override;

        virtual void SetContext(ImGuiContext* pImguiConext) override;
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
        virtual void* GetTextureID(TextureHandle texture) override;

        void LogGLError(const unsigned int& err, bool bIncludeTimeStamp);

    private:
        const int MINIMAGECOUNT = 2;
    };
}
