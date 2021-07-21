#include "RenderQueue.h"

namespace Glory
{
    void RenderQueue::EnqueueFrame(const RenderFrame& frame)
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_pRenderQueue.push(frame);
        lock.unlock();

        m_QueueCondition.notify_one();
    }

    void RenderQueue::GetNextFrame(std::function<void(const RenderFrame&)> callback)
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_QueueCondition.wait(lock, [&]() { return !m_pRenderQueue.empty(); });
        const RenderFrame& frame = m_pRenderQueue.front();
        callback(frame);
        m_pRenderQueue.pop();
        lock.unlock();
    }
}