#pragma once
#include "Module.h"

namespace Glory
{
    class GraphicsModule : public Module
    {
    public:
        GraphicsModule();
        virtual ~GraphicsModule();

    public: // Module functions


    protected:
        virtual void Initialize() = 0;
        virtual void Update() = 0;
        virtual void Draw() = 0;
    };
}