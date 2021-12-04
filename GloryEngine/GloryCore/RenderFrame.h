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
		std::vector<RenderData> ObjectsToRender;
		std::vector<CoreCamera> ActiveCameras;
	};
}
