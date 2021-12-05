#include "stdafx.h"
#include <JobManager.h>
#include "RendererModule.h"
#include <VertexHelpers.h>
#include <OGLMaterial.h>

#define _CRTDBG_MAP_ALLOC

//#define EDITOR

namespace Glory
{
    class NoRendering : public RendererModule
    {
    public:
        NoRendering() {}
        virtual ~NoRendering() {}

    protected:
        friend class GraphicsThread;
        virtual void Initialize()
        {
            FileImportSettings importSettings;
            importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
            importSettings.AddNullTerminateAtEnd = true;
            FileData* pVert = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/ScreenRenderer.vert", importSettings);
            FileData* pFrag = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/ScreenRenderer.frag", importSettings);

            std::vector<FileData*> pShaderFiles = { pVert, pFrag };
            std::vector<ShaderType> shaderTypes = { ShaderType::ST_Vertex, ShaderType::ST_Fragment };

            m_pScreenMaterial = new MaterialData(pShaderFiles, shaderTypes);
        }

        virtual void Cleanup()
        {
            delete m_pScreenMesh;
            m_pScreenMesh = nullptr;
        }

        virtual void Render(const RenderFrame& frame) override
        {
            int width, height;
            m_pEngine->GetWindowModule()->GetMainWindow()->GetDrawableSize(&width, &height);
            if (m_pRenderTexture == nullptr) m_pRenderTexture = m_pEngine->GetGraphicsModule()->GetResourceManager()->CreateRenderTexture(width, height, true);

            MeshData* pMeshData = nullptr;
            GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
            for (size_t i = 0; i < frame.ActiveCameras.size(); i++)
            {
                m_pRenderTexture->Bind();
                pGraphics->Clear();
                for (size_t j = 0; j < frame.ObjectsToRender.size(); j++)
                {
                    RenderData renderData = frame.ObjectsToRender[j];
                    if (renderData.m_pModel == nullptr) continue;
                    pMeshData = renderData.m_pModel->GetMesh(renderData.m_MeshIndex);
                    Material* pMaterial = pGraphics->UseMaterial(renderData.m_pMaterial);

                    UniformBufferObjectTest ubo;
                    ubo.model = renderData.m_World;
                    ubo.view = frame.ActiveCameras[i].m_View;
                    ubo.proj = frame.ActiveCameras[i].m_Projection;

                    pMaterial->SetUBO(ubo);
                    pMaterial->SetTexture(renderData.m_pMaterial->GetTexture());
                    pMaterial->SetProperties();
                    pGraphics->DrawMesh(pMeshData);
                }
                m_pRenderTexture->UnBind();
            }


            CreateMesh();

            glDisable(GL_DEPTH_TEST);

            pGraphics->Clear();
            glBindFramebuffer(GL_FRAMEBUFFER, NULL);
            OpenGLGraphicsModule::LogGLError(glGetError());
            glViewport(0, 0, width, height);
            OpenGLGraphicsModule::LogGLError(glGetError());

            // Set material
            OGLMaterial* pMaterial = (OGLMaterial*)pGraphics->UseMaterial(m_pScreenMaterial);
            GLTexture* pTexture = (GLTexture*)m_pRenderTexture->GetTexture();
            pMaterial->SetTexture("ScreenTexture", pTexture->GetID());

            // Draw the screen mesh
            glBindVertexArray(m_ScreenQuadVertexArrayID);
            OpenGLGraphicsModule::LogGLError(glGetError());

            // Draw the triangles !
            glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
            OpenGLGraphicsModule::LogGLError(glGetError());

            pGraphics->Swap();

            glBindVertexArray(NULL);
            OpenGLGraphicsModule::LogGLError(glGetError());

            // Reset render textures and materials
            glBindFramebuffer(GL_FRAMEBUFFER, NULL);
            glViewport(0, 0, width, height);
            glUseProgram(NULL);
            OpenGLGraphicsModule::LogGLError(glGetError());

            glEnable(GL_DEPTH_TEST);
        }

        void CreateMesh()
        {
            if (m_HasMesh) return;

            static const GLfloat g_quad_vertex_buffer_data[] = {
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
            };

            glGenVertexArrays(1, &m_ScreenQuadVertexArrayID);
            OpenGLGraphicsModule::LogGLError(glGetError());
            glBindVertexArray(m_ScreenQuadVertexArrayID);
            OpenGLGraphicsModule::LogGLError(glGetError());

            glGenBuffers(1, &m_ScreenQuadVertexbufferID);
            OpenGLGraphicsModule::LogGLError(glGetError());
            glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadVertexbufferID);
            OpenGLGraphicsModule::LogGLError(glGetError());
            glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
            OpenGLGraphicsModule::LogGLError(glGetError());

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            OpenGLGraphicsModule::LogGLError(glGetError());

            glBindBuffer(GL_ARRAY_BUFFER, NULL);
            OpenGLGraphicsModule::LogGLError(glGetError());

            glEnableVertexAttribArray(0);
            OpenGLGraphicsModule::LogGLError(glGetError());

            glBindVertexArray(NULL);
            OpenGLGraphicsModule::LogGLError(glGetError());
            m_HasMesh = true;
        }

    private:
        MeshData* m_pScreenMesh;
        RenderTexture* m_pRenderTexture = nullptr;
        MaterialData* m_pScreenMaterial;


        GLuint m_ScreenQuadVertexArrayID;
        GLuint m_ScreenQuadVertexbufferID;

        bool m_HasMesh = false;
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
