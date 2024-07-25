#pragma once
#include "RenderQueue.h"
#include "Thread.h"
#include "Camera.h"
#include "ThreadedVar.h"

#include <functional>

namespace Glory
{
	class Engine;

	class GraphicsThread
	{
	public:
		GraphicsThread(Engine* pEngine);
		virtual ~GraphicsThread();

		void Start();
		void Stop();

		RenderQueue* GetRenderQueue();

		template<typename T>
		void BindForDraw(T* pModule)
		{
			m_RenderBinds.push_back(std::bind(&T::Render, pModule, std::placeholders::_1));
			m_InitializationBinds.push_back(std::bind(&T::ThreadedInitialize, pModule));
			m_CleanupBinds.push_back(std::bind(&T::ThreadedCleanup, pModule));
		}

		template<typename T>
		void BindInitializeOnly(T* pModule)
		{
			m_InitializationBinds.push_back(std::bind(&T::ThreadedInitialize, pModule));
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

		template<typename T>
		void BindBeginAndEndRender(T* pModule)
		{
			m_BeginRenderBinds.push_back(std::bind(&T::GraphicsThreadBeginRender, pModule));
			m_EndRenderBinds.push_back(std::bind(&T::GraphicsThreadEndRender, pModule));
		}

		void Execute(std::function<void(void*)> func, void* pData);

		const bool IsInitialized() const;

	private:
		void Run();
		void OnRenderFrame(const RenderFrame& frame);

	private:
		struct ExecuteData
		{
			UUID m_UUID;
			std::function<void(void*)> m_Func;
			void* m_pData;
		};

		Thread* m_pThread;
		RenderQueue* m_pRenderQueue;
		Engine* m_pEngine;
		std::vector<std::function<void(const RenderFrame&)>> m_RenderBinds;
		std::vector<std::function<void()>> m_InitializationBinds;
		std::vector<std::function<void()>> m_CleanupBinds;
		std::vector<std::function<void()>> m_BeginRenderBinds;
		std::vector<std::function<void()>> m_EndRenderBinds;
		ThreadedVector<ExecuteData> m_Executes;
		bool m_Exit;
		bool m_Initialized;
	};
}
