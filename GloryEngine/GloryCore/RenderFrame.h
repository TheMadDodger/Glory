#pragma once
#include "GraphicsCommands.h"
#include "CommandData.h"
#include <queue>

namespace Glory
{
	struct RenderFrame
	{
	public:
		RenderFrame();

		void Enqueue(const std::any& data);

		std::vector<std::any> CommandQueue;
	};
}
