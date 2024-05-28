#include "RenderQueue.h"

namespace Glory
{
    RenderQueue::RenderQueue(uint8_t queueLimit) : QUEUE_LIMIT(queueLimit), m_Exit(false)
    {
    }

    RenderQueue::~RenderQueue()
    {
    }

    void RenderQueue::EnqueueFrame(RenderFrame&& frame)
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_pRenderQueue.push(std::move(frame));
        lock.unlock();

        //m_QueueCondition.notify_one();
    }

    void RenderQueue::GetNextFrame(std::function<void(const RenderFrame&)> callback)
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        //m_QueueCondition.wait(lock, [&]() { return !m_pRenderQueue.empty() || m_Exit; });
        if (m_Exit) return;

        if (m_pRenderQueue.empty())
        {
            lock.unlock();
            return;
        }

        const RenderFrame& frame = m_pRenderQueue.front();
        lock.unlock();
        callback(frame);
        m_pRenderQueue.pop();
    }

    bool RenderQueue::IsFull()
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        size_t queueSize = m_pRenderQueue.size();
        lock.unlock();
        return queueSize >= QUEUE_LIMIT;
    }

    void RenderQueue::Stop()
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_Exit = true;
        lock.unlock();
        //m_QueueCondition.notify_one();
    }
}