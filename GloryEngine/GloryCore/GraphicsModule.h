#pragma once
#include "Module.h"
#include "Buffer.h"
#include "GraphicsThread.h"
#include "GraphicsCommands.h"

namespace Glory
{
    class FrameStates;

    class GraphicsModule : public Module
    {
    public:
        GraphicsModule();
        virtual ~GraphicsModule();

        virtual const std::type_info& GetModuleType() override;

    public: // Graphics thread
        void StartFrame();
        void EnqueueCommand(const std::any& data);
        void EndFrame();

    public: // Getters
        GraphicsThread* GetGraphicsThread();
        FrameStates* GetFrameStates();

    public: // Module functions
        Buffer* CreateVertexBuffer(uint32_t bufferSize);

    protected: // Internal functions
        virtual Buffer* CreateVertexBuffer_Internal(uint32_t bufferSize) = 0;

    protected:
        virtual void OnInitialize() = 0;
        virtual void OnCleanup() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnDraw() = 0;

        virtual void RegisterCommands();
        virtual FrameStates* CreateFrameStates();

    private:
        virtual void Initialize() override;
        virtual void Cleanup() override;
        virtual void Update() override;
        virtual void Draw() override;

    private:
        GraphicsThread* m_pGraphicsThread;
        RenderFrame m_PreparingFrame;
        FrameStates* m_pFrameStates;
    };
}