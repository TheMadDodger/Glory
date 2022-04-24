#pragma once
#include <BaseEditorExtension.h>
#include <ProjectSpace.h>
#include <GloryMonoScipting.h>
#include <ticpp.h>
#include <ObjectMenu.h>

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

        static void UpdateCSharpProjectFile();

        static void ReloadCSharpDocument(const std::string& projectPath);
        static void CreateCSharpDocument(const std::string& projectPath);

    private:
        GloryMonoScipting* m_pScriptingModule;
        static ticpp::Document m_SolutionDocument;
    };
}