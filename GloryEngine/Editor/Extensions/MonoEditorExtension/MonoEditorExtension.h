#pragma once
#include <BaseEditorExtension.h>
#include <ProjectSpace.h>
#include <GloryMonoScipting.h>
#include <ObjectMenu.h>
#include <Glory.h>
#include <ResourceMeta.h>

extern "C" GLORY_EDITOR_EXTENSION_API Glory::Editor::BaseEditorExtension * LoadExtension();
extern "C" GLORY_EDITOR_EXTENSION_API void SetContext(Glory::GloryContext * pContext, ImGuiContext * pImGUIContext);

namespace Glory::Editor
{
    class MonoEditorExtension : public BaseEditorExtension
    {
    public:
        MonoEditorExtension();
        virtual ~MonoEditorExtension();

        static void OpenCSharpProject();

        static void OpenFile(const std::filesystem::path& path);

    private:
        virtual void RegisterEditors() override;
        static void FindVisualStudioPath();
        static bool FindVisualStudioPath(const std::filesystem::path& path);
        static bool FindMSBuild(std::filesystem::path& path);

        static void OnProjectClose(ProjectSpace* pProject);
        static void OnProjectOpen(ProjectSpace* pProject);

        static void OnCreateScript(Object* pObject, const ObjectMenuType& menuType);
        static void OnOpenCSharpProject(Object* pObject, const ObjectMenuType& menuType);

        static void CopyEngineAssemblies(ProjectSpace* pProject);
        static void GeneratePremakeFile(ProjectSpace* pProject);
        static void GenerateBatchFile(ProjectSpace* pProject);
        static void RunGenerateProjectFilesBatch(ProjectSpace* pProject);

        static void CompileProject();
        static void ReloadAssembly();

        static void AssetCallback(UUID uuid, const ResourceMeta& meta, Resource*);

        void Preferences();

    private:
        static std::filesystem::path m_VisualStudioPath;
    };
}