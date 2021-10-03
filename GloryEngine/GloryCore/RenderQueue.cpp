#include "RenderQueue.h"

namespace Glory
{
    RenderQueue::RenderQueue(uint8_t queueLimit) : QUEUE_LIMIT(queueLimit)
    {
    }

    RenderQueue::~RenderQueue()
    {
    }

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

    bool RenderQueue::IsFull()
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        size_t queueSize = m_pRenderQueue.size();
        lock.unlock();
        return queueSize >= QUEUE_LIMIT;
    }
}