#pragma once
#include <BaseEditorExtension.h>
#include <ProjectSpace.h>
#include <GloryMonoScipting.h>
#include <ticpp.h>

namespace Glory::Editor
{
    class MonoEditorExtension : public BaseEditorExtension
    {
    public:
        MonoEditorExtension();
        virtual ~MonoEditorExtension();

    private:
        virtual void RegisterEditors() override;

        static void OnProjectClose(ProjectSpace* pProject);
        static void OnProjectOpen(ProjectSpace* pProject);

    private:
        GloryMonoScipting* m_pScriptingModule;
        static ticpp::Document m_SolutionDocument;
    };
}