#include "RenderFrame.h"

namespace Glory
{
    RenderFrame::RenderFrame(size_t maxLigts):
        ActiveLights(maxLigts), LightViews(maxLigts), LightProjections(maxLigts)
    {
    }

    RenderFrame::~RenderFrame()
    {
        Picking.clear();
        ActiveLights.reset();
        ActiveLightIDs.clear();
        LightViews.reset();
        LightProjections.reset();
    }

    void RenderFrame::Reset()
    {
        Picking.clear();
        ActiveLights.reset();
        ActiveLightIDs.clear();
        LightViews.reset();
        LightProjections.reset();
    }
}