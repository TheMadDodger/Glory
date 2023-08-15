#include "RenderFrame.h"

namespace Glory
{
    RenderFrame::RenderFrame() : ObjectsToRender(std::vector<RenderData>()),
        ActiveLights(1000)
    {
    }
}