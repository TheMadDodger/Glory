#pragma once
#include <queue>
#include "RenderData.h"

namespace Glory
{
	struct RenderFrame
	{
	public:
		RenderFrame();
		std::vector<RenderData> ObjectsToRender;
	};
}
