#include "stdafx.h"
#include <JobManager.h>
#include "RendererModule.h"
#include <ResourceMeta.h>

#define _CRTDBG_MAP_ALLOC

#define EDITOR

namespace Glory
{
    class NoRendering : public RendererModule
    {
    public:
        NoRendering() {}
        virtual ~NoRendering() {}

    protected:
        friend class GraphicsThread;
        virtual void Initialize() {}
        virtual void Cleanup() {}
        virtual void Render(const RenderFrame& frame) override
        {
            GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
            //pGraphics->Clear();
            //for (size_t i = 0; i < frame.ObjectsToRender.size(); i++)
            //{
            //    RenderData renderData = frame.ObjectsToRender[i];
            //    if (renderData.m_pModel == nullptr) continue;
            //    MeshData* pMesh = renderData.m_pModel->GetMesh(renderData.m_MeshIndex);
            //    Material* pMaterial = pGraphics->UseMaterial(renderData.m_pMaterial);
            //    UniformBufferObjectTest ubo = renderData.m_UBO;
            //    pMaterial->SetUBO(renderData.m_UBO);
            //    pMaterial->SetTexture(renderData.m_pMaterial->GetTexture());
            //    pMaterial->SetProperties();
            //    pGraphics->DrawMesh(pMesh);
            //}
            //pGraphics->Swap();
        }
    };
}

#ifdef EDITOR

#include <EditorSDLWindowImpl.h>
#include <EditorVulkanRenderImpl.h>
#include <EditorOpenGLRenderImpl.h>
#include <EditorApplication.h>

using namespace Glory::Editor;

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = "Glory Editor";
        windowCreateInfo.Width = 2560;
        windowCreateInfo.Height = 1300;
        windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

        std::vector<Glory::Module*> optionalModules = { new Glory::SDLImageLoaderModule(), new Glory::ASSIMPModule(), new Glory::FileLoaderModule(), new Glory::GLSLShaderLoader()};

        Glory::EngineCreateInfo createInfo;
        createInfo.pWindowModule = new Glory::SDLWindowModule(windowCreateInfo);
        createInfo.pScenesModule = new Glory::EntitySceneScenesModule();
        createInfo.pRenderModule = new Glory::NoRendering();
        createInfo.pGraphicsModule = new Glory::OpenGLGraphicsModule();
        createInfo.OptionalModuleCount = static_cast<uint32_t>(optionalModules.size());
        createInfo.pOptionalModules = optionalModules.data();
        Glory::Engine* pEngine = Glory::Engine::CreateEngine(createInfo);

        Glory::GameSettings gameSettings;
        gameSettings.pEngine = pEngine;
        gameSettings.pGameState = new Glory::GameState();
        Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);
        pGame.Initialize();

        EditorApplication editorApp;
        editorApp.Initialize<EditorSDLWindowImpl, EditorOpenGLRenderImpl>(pGame);
        editorApp.Run();
        editorApp.Destroy();

        pGame.Destroy();
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

        std::vector<Glory::Module*> optionalModules = { new Glory::SDLImageLoaderModule(), new Glory::ASSIMPModule(), new Glory::FileLoaderModule(), new Glory::GLSLShaderLoader() };

        Glory::EngineCreateInfo createInfo;
        createInfo.pWindowModule = new Glory::SDLWindowModule(windowCreateInfo);
        createInfo.pScenesModule = new Glory::EntitySceneScenesModule();
        createInfo.pRenderModule = new Glory::NoRendering();
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
