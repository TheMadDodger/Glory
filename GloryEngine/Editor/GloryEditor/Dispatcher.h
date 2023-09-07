#pragma once
#include <functional>
#include <map>
#include <UUID.h>

namespace Glory::Editor
{
	template<typename T>
	struct Dispatcher
	{
	public:
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
	};
}
