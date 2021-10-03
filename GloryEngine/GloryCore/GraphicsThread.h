#pragma once
#include "RenderQueue.h"
#include "Thread.h"

namespace Glory
{
	class GraphicsThread
	{
	public:
		GraphicsThread();
		virtual ~GraphicsThread();

		void Start();
		void Stop();

		RenderQueue* GetRenderQueue();

		template<typename T>
		void Bind(T* pModule)
		{
			m_RenderBinds.push_back(std::bind(&T::Render, pModule, std::placeholders::_1));
			m_InitializationBinds.push_back(std::bind(&T::ThreadedInitialize, pModule));
		}

	private:
		void Run();

		void OnRenderFrame(const RenderFrame& frame);

	private:
		Thread* m_pThread;
		RenderQueue* m_pRenderQueue;
		std::vector<std::function<void(const RenderFrame&)>> m_RenderBinds;
		std::vector<std::function<void()>> m_InitializationBinds;
	};
}
