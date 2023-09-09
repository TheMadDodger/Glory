#pragma once
#include <functional>
#include <map>
#include <UUID.h>
#include <mutex>
#include <vector>

namespace Glory::Editor
{
	template<typename T>
	struct Dispatcher
	{
	public:
		void Enqueue(const T& arg)
		{
			std::unique_lock lock{m_Mutex};
			m_Enqueued.push_back(arg);
		}

		void Enqueue(T&& arg)
		{
			std::unique_lock lock{m_Mutex};
			m_Enqueued.push_back(std::move(arg));
		}

		void Flush()
		{
			std::unique_lock lock{m_Mutex};
			for (size_t i = 0; i < m_Enqueued.size(); ++i)
			{
				Dispatch(m_Enqueued[i]);
			}
			m_Enqueued.clear();
		}

		void Dispatch(const T& arg)
		{
			for (auto itor = m_Observers.begin(); itor != m_Observers.end(); ++itor)
			{
				itor->second(arg);
			}
		}

		UUID AddListener(std::function<void(const T&)> listener)
		{
			const UUID uuid = UUID();
			m_Observers.emplace(uuid, listener);
			return uuid;
		}

		void RemoveListener(UUID& id)
		{
			m_Observers.erase(id);
			id = 0;
		}

	private:
		std::map<UUID, std::function<void(const T&)>> m_Observers;
		std::vector<T> m_Enqueued;
		std::mutex m_Mutex;
	};
}
