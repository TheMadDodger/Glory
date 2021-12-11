#pragma once
#include <queue>
#include "RenderData.h"
#include "Camera.h"

namespace Glory
{
	struct RenderFrame
	{
	public:
		RenderFrame();

	public:
		std::vector<RenderData> ObjectsToRender;
		std::vector<Camera*> ActiveCameras;
	};
}
