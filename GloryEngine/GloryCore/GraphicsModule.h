#pragma once
#include "Module.h"
#include "Buffer.h"
#include "GraphicsThread.h"

namespace Glory
{
    class FrameStates;

    class GraphicsModule : public Module
    {
    public:
        GraphicsModule();
        virtual ~GraphicsModule();

        virtual const std::type_info& GetModuleType() override;

    public: // Commands
        virtual void Clear() = 0;
        virtual void Swap() = 0;
        virtual void DrawMesh(MeshData* pMeshData) = 0;

    public: // Getters
        FrameStates* GetFrameStates();

    public: // Module functions
        Buffer* CreateVertexBuffer(uint32_t bufferSize);

    protected: // Internal functions
        virtual Buffer* CreateVertexBuffer_Internal(uint32_t bufferSize) = 0;

    protected:
        virtual void OnInitialize() = 0;
        virtual void OnThreadedInitialize() {}
        virtual void OnCleanup() = 0;

        virtual FrameStates* CreateFrameStates();

    private:
        virtual void Initialize() override;
        virtual void Cleanup() override;

    private:
        friend class RendererModule;
        RenderFrame m_PreparingFrame;
        FrameStates* m_pFrameStates;
    };
}