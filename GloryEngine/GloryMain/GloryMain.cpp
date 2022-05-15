#include "stdafx.h"
#include "RendererModule.h"
#include <JobManager.h>
#include <VertexHelpers.h>
#include <OGLMaterial.h>
#include <ResourceMeta.h>
#include <ClusteredRendererModule.h>
#include <ShaderSourceLoaderModule.h>
#include <MaterialLoaderModule.h>
#include <MaterialInstanceLoaderModule.h>
#include <GloryMonoScipting.h>

#define _CRTDBG_MAP_ALLOC

#define EDITOR

#ifdef EDITOR

#include <EditorSDLWindowImpl.h>
#include <EditorVulkanRenderImpl.h>
#include <EditorOpenGLRenderImpl.h>
#include <EditorApplication.h>
#include <EntityScenesEditorExtension.h>
#include <MonoEditorExtension.h>
#include <yaml-cpp/yaml.h>

using namespace Glory::Editor;

typedef void(__cdecl* LoadExtensionProc)(Glory::Engine*);

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = "Glory Editor";
        windowCreateInfo.Width = 2560;
        windowCreateInfo.Height = 1300;
        windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

        std::vector<Glory::Module*> optionalModules = {
            new Glory::SDLImageLoaderModule(),
            new Glory::ASSIMPModule(),
            new Glory::FileLoaderModule(),
            new Glory::MaterialLoaderModule(),
            new Glory::MaterialInstanceLoaderModule(),
            new Glory::ShaderSourceLoaderModule(),
        };

        std::vector<Glory::ScriptingModule*> scriptingModules = {
            new Glory::GloryMonoScipting(),
        };

        Glory::EngineCreateInfo createInfo;
        createInfo.pWindowModule = new Glory::SDLWindowModule(windowCreateInfo);
        createInfo.pScenesModule = new Glory::EntitySceneScenesModule();
        createInfo.pRenderModule = new Glory::ClusteredRendererModule();
        createInfo.pGraphicsModule = new Glory::OpenGLGraphicsModule();
        createInfo.OptionalModuleCount = static_cast<uint32_t>(optionalModules.size());
        createInfo.pOptionalModules = optionalModules.data();
        createInfo.ScriptingModulesCount = static_cast<uint32_t>(scriptingModules.size());
        createInfo.pScriptingModules = scriptingModules.data();
        Glory::Engine* pEngine = Glory::Engine::CreateEngine(createInfo);

        HMODULE lib = LoadLibraryA("./GloryEntitiesMonoExtender.dll");
        LoadExtensionProc loadProc = (LoadExtensionProc)GetProcAddress(lib, "LoadExtension");
        (loadProc)(pEngine);

        Glory::GameSettings gameSettings;
        gameSettings.pEngine = pEngine;
        gameSettings.pGameState = new Glory::GameState();
        gameSettings.ApplicationType = Glory::ApplicationType::AT_Editor;
        Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);
        pGame.Initialize();

        std::vector<Glory::Editor::BaseEditorExtension*> editorExtensions= 
        {
            new Glory::Editor::EntityScenesEditorExtension(),
            new Glory::Editor::MonoEditorExtension()
        };

        Glory::EditorCreateInfo editorCreateInfo;
        editorCreateInfo.ExtensionsCount = static_cast<uint32_t>(editorExtensions.size());
        editorCreateInfo.pExtensions = editorExtensions.data();

        EditorApplication editorApp(editorCreateInfo);
        editorApp.Initialize<EditorSDLWindowImpl, EditorOpenGLRenderImpl>(pGame);
        editorApp.Run(pGame);
        editorApp.Destroy();

        pGame.Destroy();

        FreeLibrary(lib);
    }

    _CrtDumpMemoryLeaks();

    return 0;
}

#else

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = "Glory Game";
        windowCreateInfo.Width = 1280;
        windowCreateInfo.Height = 720;
        windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL; //SDL_WINDOW_VULKAN;

        std::vector<Glory::Module*> optionalModules = {
            new Glory::SDLImageLoaderModule(),
            new Glory::ASSIMPModule(),
            new Glory::FileLoaderModule(),
            new Glory::MaterialLoaderModule(),
            new Glory::MaterialInstanceLoaderModule(),
            new Glory::ShaderSourceLoaderModule(),
        };

        Glory::EngineCreateInfo createInfo;
        createInfo.pWindowModule = new Glory::SDLWindowModule(windowCreateInfo);
        createInfo.pScenesModule = new Glory::EntitySceneScenesModule();
        createInfo.pRenderModule = new Glory::ClusteredRendererModule();
        createInfo.pGraphicsModule = new Glory::OpenGLGraphicsModule();
        createInfo.OptionalModuleCount = static_cast<uint32_t>(optionalModules.size());
        createInfo.pOptionalModules = optionalModules.data();
        Glory::Engine* pEngine = Glory::Engine::CreateEngine(createInfo);

        Glory::GameSettings gameSettings;
        gameSettings.pEngine = pEngine;
        gameSettings.pGameState = new Glory::GameState();
        Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);

        pGame.RunGame();
        pGame.Destroy();
    }

    _CrtDumpMemoryLeaks();
}


#endif // EDITOR
