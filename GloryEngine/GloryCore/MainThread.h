#pragma once
#include <vector>
#include <functional>

namespace Glory
{
	class MainThread
	{
	public:
		MainThread();
		virtual ~MainThread();

		template<typename T>
		void Bind(T* pModule)
		{
			m_MainUpdateBinds.push_back(std::bind(&T::MainUpdate, pModule));
		}

	private:
		void Update();

	private:
		friend class Engine;
		std::vector<std::function<void()>> m_MainUpdateBinds;
	};
}
