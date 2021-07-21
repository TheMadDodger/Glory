#pragma once
#include "GraphicsCommands.h"
#include "CommandData.h"
#include <queue>

namespace Glory
{
	struct RenderFrame
	{
		template<typename T>
		void Enqueue(T data)
		{
			CommandQueue.push_back(CommandData(typeid(T), data));
		}

		std::vector<CommandData> CommandQueue;
	};
}
