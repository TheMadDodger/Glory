#pragma once
#include "Module.h"

namespace Glory
{
    class ScenesModule : public Module
    {
    public:
        virtual const std::type_info& GetModuleType() override;

    protected:
        virtual void Initialize() = 0;
        virtual void Cleanup() = 0;
        virtual void Tick() = 0;
        virtual void OnPaint() = 0;

    private:
        void Paint();

    private:
        friend class GameThread;
    };
}
