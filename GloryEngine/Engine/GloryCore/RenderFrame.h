#pragma once
#include <queue>
#include "RenderData.h"
#include "CameraRef.h"
#include "LightData.h"

namespace Glory
{
	struct RenderFrame
	{
	public:
		RenderFrame();

	public:
		std::vector<RenderData> ObjectsToRender;
		std::vector<CameraRef> ActiveCameras;
		std::vector<PointLight> ActiveLights;
	};
}
