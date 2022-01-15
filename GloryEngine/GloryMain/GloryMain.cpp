#include "stdafx.h"
#include "RendererModule.h"
#include <JobManager.h>
#include <VertexHelpers.h>
#include <OGLMaterial.h>
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
        }

        virtual void OnRender(CameraRef camera, const RenderData& renderData) override
        {
            MeshData* pMeshData = nullptr;
            GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
            if (renderData.m_pModel == nullptr) return;
            pMeshData = renderData.m_pModel->GetMesh(renderData.m_MeshIndex);
            Material* pMaterial = pGraphics->UseMaterial(renderData.m_pMaterial);

            UniformBufferObjectTest ubo;
            ubo.model = renderData.m_World;
            ubo.view = camera.GetView();
            ubo.proj = camera.GetProjection();

            pMaterial->SetUBO(ubo);
            pMaterial->SetProperties();
            pMaterial->SetPropertiesExtra();
            pGraphics->DrawMesh(pMeshData);
        }

        virtual void OnDoScreenRender(RenderTexture* pRenderTexture) override
        {
            GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();

            Window* pWindow = m_pEngine->GetWindowModule()->GetMainWindow();

            int width, height;
            pWindow->GetDrawableSize(&width, &height);

            CreateMesh();
            
            glDisable(GL_DEPTH_TEST);

            //glBindFramebuffer(GL_FRAMEBUFFER, NULL);
            //OpenGLGraphicsModule::LogGLError(glGetError());
            glViewport(0, 0, width, height);
            OpenGLGraphicsModule::LogGLError(glGetError());
            
            // Set material
            OGLMaterial* pMaterial = (OGLMaterial*)pGraphics->UseMaterial(m_pScreenMaterial);
            GLTexture* pTexture = (GLTexture*)pRenderTexture->GetTexture();
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
            //glBindFramebuffer(GL_FRAMEBUFFER, NULL);
            //glViewport(0, 0, width, height);
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
#include <EntityScenesEditorExtension.h>

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

        std::vector<Glory::Editor::BaseEditorExtension*> editorExtensions = { new Glory::Editor::EntityScenesEditorExtension() };

        Glory::EditorCreateInfo editorCreateInfo;
        editorCreateInfo.ExtensionsCount = static_cast<uint32_t>(editorExtensions.size());
        editorCreateInfo.pExtensions = editorExtensions.data();

        EditorApplication editorApp(editorCreateInfo);
        editorApp.Initialize<EditorSDLWindowImpl, EditorOpenGLRenderImpl>(pGame);
        editorApp.Run(pGame);
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
