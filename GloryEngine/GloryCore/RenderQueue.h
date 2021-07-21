#pragma once
#include "RenderFrame.h"
#include <queue>
#include <mutex>
#include <functional>

namespace Glory
{
	class RenderQueue
	{
	public:
		void EnqueueFrame(const RenderFrame& frame);
		void GetNextFrame(std::function<void(const RenderFrame&)> callback);

	private:
		std::queue<RenderFrame> m_pRenderQueue;
		std::condition_variable m_QueueCondition;
		std::mutex m_QueueMutex;
	};
}
