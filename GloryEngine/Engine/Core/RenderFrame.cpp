#include "RenderFrame.h"

namespace Glory
{
    RenderFrame::RenderFrame() : ObjectsToRender(std::vector<RenderData>()), ActiveLights(1000)
    {
    }

    RenderFrame::~RenderFrame()
    {
        ObjectsToRender.clear();
        ActiveCameras.clear();
    }
}