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
		RenderQueue(uint8_t queueLimit);
		virtual ~RenderQueue();

	public:
		void EnqueueFrame(RenderFrame&& frame);
		void GetNextFrame(std::function<void(const RenderFrame&)> callback);
		//void IsEmpty();

		bool IsFull();

		void Stop();

	private:
		std::queue<RenderFrame> m_pRenderQueue;
		//std::condition_variable m_QueueCondition;
		std::mutex m_QueueMutex;
		bool m_Exit;
		const uint8_t QUEUE_LIMIT;
	};
}
