#include "RenderFrame.h"

namespace Glory
{
    RenderFrame::RenderFrame(size_t maxLigts):
        ActiveLights(maxLigts), LightSpaceTransforms(maxLigts)
    {
    }

    RenderFrame::~RenderFrame()
    {
        Picking.clear();
        ActiveLights.reset();
        LightSpaceTransforms.reset();
    }

    void RenderFrame::Reset()
    {
        Picking.clear();
        ActiveLights.reset();
        LightSpaceTransforms.reset();
        ActiveLightIDs.clear();
    }
}