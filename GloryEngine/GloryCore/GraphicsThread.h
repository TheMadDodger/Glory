#pragma once
#include "RenderQueue.h"
#include "Thread.h"
#include "Camera.h"

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
			m_CleanupBinds.push_back(std::bind(&T::ThreadedCleanup, pModule));
		}

		template<typename T>
		void BindRenderOnly(T* pModule)
		{
			m_RenderBinds.push_back(std::bind(&T::Render, pModule, std::placeholders::_1));
		}

		template<typename T>
		void BindNoRender(T* pModule)
		{
			m_InitializationBinds.push_back(std::bind(&T::ThreadedInitialize, pModule));
			m_CleanupBinds.push_back(std::bind(&T::ThreadedCleanup, pModule));
		}

	private:
		void Run();

		void OnRenderFrame(const RenderFrame& frame);

	private:
		Thread* m_pThread;
		RenderQueue* m_pRenderQueue;
		std::vector<std::function<void(const RenderFrame&)>> m_RenderBinds;
		std::vector<std::function<void()>> m_InitializationBinds;
		std::vector<std::function<void()>> m_CleanupBinds;
		bool m_Exit;
	};
}
