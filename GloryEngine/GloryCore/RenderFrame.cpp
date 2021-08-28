#include "RenderFrame.h"

namespace Glory
{
    RenderFrame::RenderFrame() : CommandQueue(std::vector<std::any>())
    {
    }

    void RenderFrame::Enqueue(const std::any& data)
    {
        CommandQueue.push_back(data);
    }
}