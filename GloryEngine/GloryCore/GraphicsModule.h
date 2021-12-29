#pragma once
#include "Module.h"
#include "GPUResourceManager.h"

namespace Glory
{
    class FrameStates;

    class GraphicsModule : public Module
    {
    public:
        GraphicsModule();
        virtual ~GraphicsModule();

        virtual const std::type_info& GetModuleType() override;

        int GetLastDrawCalls();
        int GetLastVertexCount();
        int GetLastTriangleCount();

    public: // Commands
        virtual void Clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) = 0;
        virtual void Swap() = 0;
        virtual Material* UseMaterial(MaterialData* pMaterialData) = 0;

        void DrawMesh(MeshData* pMeshData);


    public: // Getters
        FrameStates* GetFrameStates();
        GPUResourceManager* GetResourceManager();

    protected:
        virtual void OnInitialize() = 0;
        virtual void OnCleanup() = 0;
        virtual void ThreadedInitialize() {}
        virtual void ThreadedCleanup();

        virtual void OnDrawMesh(MeshData* pMeshData) = 0;

        virtual FrameStates* CreateFrameStates();
        virtual GPUResourceManager* CreateGPUResourceManager() = 0;

    private:
        virtual void Initialize() override;
        virtual void Cleanup() override;

        virtual void OnGraphicsThreadFrameStart() override;
        virtual void OnGraphicsThreadFrameEnd() override;

    private:
        friend class GraphicsThread;
        FrameStates* m_pFrameStates;
        GPUResourceManager* m_pResourceManager;

        int m_CurrentDrawCalls;
        int m_LastDrawCalls;
        int m_LastVertices;
        int m_CurrentVertices;
        int m_LastTriangles;
        int m_CurrentTriangles;
    };
}