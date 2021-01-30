#pragma once
#include "Module.h"
#include "Buffer.h"

namespace Glory
{
    class GraphicsModule : public Module
    {
    public:
        GraphicsModule();
        virtual ~GraphicsModule();

    public: // Module functions
        Buffer* CreateVertexBuffer(uint32_t bufferSize);

    protected: // Internal functions
        virtual Buffer* CreateVertexBuffer_Internal(uint32_t bufferSize) = 0;

    protected:
        virtual void Initialize() = 0;
        virtual void Update() = 0;
        virtual void Draw() = 0;
    };
}