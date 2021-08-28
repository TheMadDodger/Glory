#pragma once
#include "GraphicsCommandHandler.h"
#include "GraphicsCommands.h"

namespace Glory
{
    class TestCommandHandler : public GraphicsCommandHandler<TestCommand>
    {
    private:
        virtual void OnInvoke(TestCommand commandData) override;
    };
}
