#pragma once
#include <BaseEditorExtension.h>
#include <ProjectSpace.h>
#include <ObjectMenu.h>
#include <Glory.h>
#include <ResourceMeta.h>
#include <IPlayModeHandler.h>

EXTENSION_H

namespace Glory
{
    class GloryMonoScipting;
}

namespace Glory::Editor
{
    struct AssetCallbackData;

    class MonoEditorExtension : public BaseEditorExtension, public IPlayModeHandler
    {
    public:
        virtual const char* ModuleName() override { return "Mono Scripting"; };
        virtual void HandleBeforeStart(Module* pModule) override {};
        virtual void HandleStart(Module* pModule) override {};
        virtual void HandleStop(Module* pModule) override;
        virtual void HandleUpdate(Module* pModule) override {};

        virtual void OnBeginPackage(const std::filesystem::path& path) override;
        virtual void OnGenerateConfigExec(std::ofstream& stream) override;
        virtual void OnEndPackage(const std::filesystem::path& path) override;

    public:
        MonoEditorExtension();
        virtual ~MonoEditorExtension();

        static void OpenCSharpProject();

        static void OpenFile(const std::filesystem::path& path);

    private:
        virtual void Initialize() override;
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

        static void CompileProject(ProjectSpace* pProject, bool release=false);
        static void ReloadAssembly(ProjectSpace* pProject);

        static void AssetCallback(const AssetCallbackData& callback);

        void Preferences();

    private:
        static GloryMonoScipting* m_pMonoScriptingModule;
    };
}