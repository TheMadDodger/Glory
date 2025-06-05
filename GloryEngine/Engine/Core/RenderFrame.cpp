#include "RenderFrame.h"

namespace Glory
{
    RenderFrame::RenderFrame(size_t maxLigts) : ObjectsToRender(std::vector<RenderData>()),
        ActiveLights(maxLigts), LightSpaceTransforms(maxLigts)
    {
    }

    RenderFrame::~RenderFrame()
    {
        ObjectsToRender.clear();
        ActiveCameras.clear();
        TextsToRender.clear();
        ObjectsToRenderLate.clear();
        Picking.clear();
        ActiveLights.reset();
        LightSpaceTransforms.reset();
    }

    void RenderFrame::Reset()
    {
        ObjectsToRender.clear();
        ActiveCameras.clear();
        TextsToRender.clear();
        ObjectsToRenderLate.clear();
        Picking.clear();
        ActiveLights.reset();
        LightSpaceTransforms.reset();
        ActiveLightIDs.clear();
    }
}