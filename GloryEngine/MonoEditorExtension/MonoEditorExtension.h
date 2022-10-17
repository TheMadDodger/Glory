#pragma once
#include <BaseEditorExtension.h>
#include <ProjectSpace.h>
#include <GloryMonoScipting.h>
#include <ObjectMenu.h>
#include <Glory.h>

extern "C" GLORY_API Glory::Editor::BaseEditorExtension * LoadExtension();
extern "C" GLORY_API void SetContext(Glory::GloryContext * pContext, ImGuiContext * pImGUIContext);

namespace Glory::Editor
{
    class MonoEditorExtension : public BaseEditorExtension
    {
    public:
        MonoEditorExtension();
        virtual ~MonoEditorExtension();

        static void OpenCSharpProject();

    private:
        virtual void RegisterEditors() override;

        static void OnProjectClose(ProjectSpace* pProject);
        static void OnProjectOpen(ProjectSpace* pProject);

        static void OnCreateScript(Object* pObject, const ObjectMenuType& menuType);
        static void OnOpenCSharpProject(Object* pObject, const ObjectMenuType& menuType);

        static void CopyEngineAssemblies(ProjectSpace* pProject);
        static void GeneratePremakeFile(ProjectSpace* pProject);
        static void GenerateBatchFile(ProjectSpace* pProject);
        static void RunGenerateProjectFilesBatch(ProjectSpace* pProject);

    private:
        GloryMonoScipting* m_pScriptingModule;
    };
}