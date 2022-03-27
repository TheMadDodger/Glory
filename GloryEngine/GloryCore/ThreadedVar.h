#pragma once
#include <mutex>
#include <vector>
#include <unordered_map>
#include <functional>
#include <queue>

namespace Glory
{
	struct BaseThreadedVar
	{
	protected:
		std::mutex m_Mutex;
	};

	template<typename _Ty>
	struct ThreadedVector : public BaseThreadedVar
	{
	public:
		void push_back(const _Ty& val)
		{
			m_Mutex.lock();
			m_Data.push_back(val);
			m_Mutex.unlock();
		}

		void push_back(_Ty&& val)
		{
			m_Mutex.lock();
			m_Data.push_back(val);
			m_Mutex.unlock();
		}

		const _Ty& operator[](const size_t pos) noexcept
		{
			m_Mutex.lock();
			const _Ty& data = m_Data[pos];
			m_Mutex.unlock();
			return data;
		}

		void Set(const _Ty& value, const size_t pos)
		{
			m_Mutex.lock();
			m_Data[pos] = value;
			m_Mutex.unlock();
		}

		void ForEach(std::function<void(const _Ty& value)> callback)
		{
			m_Mutex.lock();
			for (size_t i = 0; i < m_Data.size(); i++)
			{
				callback(m_Data[i]);
			}
			m_Mutex.unlock();
		}

		void ForEachClear(std::function<void(const _Ty& value)> callback)
		{
			m_Mutex.lock();
			for (size_t i = 0; i < m_Data.size(); i++)
			{
				callback(m_Data[i]);
			}
			m_Data.clear();
			m_Mutex.unlock();
		}

		size_t Size() const
		{
			m_Mutex.lock();
			size_t size = m_Data.size();
			m_Mutex.unlock();
			return size;
		}

		void Clear()
		{
			m_Mutex.lock();
			m_Data.clear();
			m_Mutex.unlock();
		}

		bool Contains(const _Ty& val)
		{
			bool result;
			m_Mutex.lock();
			auto it = std::find(m_Data.begin(), m_Data.end(), val);
			result = it != m_Data.end();
			m_Mutex.unlock();
			return result;
		}

		void Erase(const _Ty& val)
		{
			m_Mutex.lock();
			auto it = std::find(m_Data.begin(), m_Data.end(), val);
			if (it != m_Data.end()) m_Data.erase(it);
			m_Mutex.unlock();
		}

		void Do(size_t index, std::function<void(_Ty* value)> callback)
		{
			m_Mutex.lock();
			callback(&m_Data[index]);
			m_Mutex.unlock();
		}

		void Do(size_t index, std::function<void(const _Ty& value)> callback)
		{
			m_Mutex.lock();
			callback(m_Data[index]);
			m_Mutex.unlock();
		}

	private:
		std::vector<_Ty> m_Data;
	};

	template<typename _Kty, typename _Ty, class _Hasher = std::hash<_Kty>>
	struct ThreadedUMap : public BaseThreadedVar
	{
	public:
		_Ty operator[](const _Kty& key)
		{
			m_Mutex.lock();
			_Ty value = m_Data[key];
			m_Mutex.unlock();
			return value;
		}

		bool Contains(const _Kty& key)
		{
			m_Mutex.lock();
			bool found = m_Data.find(key) != m_Data.end();
			m_Mutex.unlock();
			return found;
		}

		void Set(const _Kty& key, const _Ty& value)
		{
			m_Mutex.lock();
			m_Data[key] = value;
			m_Mutex.unlock();
		}

		void Erase(const _Kty& key)
		{
			m_Mutex.lock();
			m_Data.erase(key);
			m_Mutex.unlock();
		}

		void Do(const _Kty& key, std::function<void(_Ty* value)> callback)
		{
			m_Mutex.lock();
			callback(&m_Data[key]);
			m_Mutex.unlock();
		}

		void Do(const _Kty& key, std::function<void(const _Ty& value)> callback)
		{
			m_Mutex.lock();
			callback(m_Data[key]);
			m_Mutex.unlock();
		}

		size_t Size()
		{
			m_Mutex.lock();
			size_t size = m_Data.size();
			m_Mutex.unlock();
			return size;
		}

		void ForEach(std::function<void(const _Kty& key, const _Ty& value)> callback)
		{
			m_Mutex.lock();
			for (auto it = m_Data.begin(); it != m_Data.end(); it++)
			{
				callback(it->first, it->second);
			}
			m_Mutex.unlock();
		}

		void ForEach(std::function<void(_Ty value)> callback)
		{
			m_Mutex.lock();
			for (auto it = m_Data.begin(); it != m_Data.end(); it++)
			{
				callback(it->first, it->second);
			}
			m_Mutex.unlock();
		}

		void Clear()
		{
			m_Mutex.lock();
			m_Data.clear();
			m_Mutex.unlock();
		}

	private:
		std::unordered_map<_Kty, _Ty, _Hasher> m_Data;
	};

	template<typename _Ty>
	struct ThreadedQueue : public BaseThreadedVar
	{
	public:
		void push(const _Ty& val)
		{
			m_Mutex.lock();
			m_Data.push(val);
			m_Mutex.unlock();
		}

		void push(_Ty&& val)
		{
			m_Mutex.lock();
			m_Data.push(val);
			m_Mutex.unlock();
		}

		size_t Size() const
		{
			m_Mutex.lock();
			size_t size = m_Data.size();
			m_Mutex.unlock();
			return size;
		}

		void Clear()
		{
			m_Mutex.lock();
			m_Data.clear();
			m_Mutex.unlock();
		}

		bool Pop(_Ty& value)
		{
			bool result;
			m_Mutex.lock();
			result = m_Data.size() > 0;
			if (result)
			{
				value = m_Data.front();
				m_Data.pop();
			}
			m_Mutex.unlock();
			return result;
		}

	private:
		std::queue<_Ty> m_Data;
	};
}
