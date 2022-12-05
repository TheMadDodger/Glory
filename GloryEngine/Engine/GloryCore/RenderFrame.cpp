#include "RenderFrame.h"

namespace Glory
{
    RenderFrame::RenderFrame() : ObjectsToRender(std::vector<RenderData>()), ActiveLights(std::vector<PointLight>())
    {
    }
}