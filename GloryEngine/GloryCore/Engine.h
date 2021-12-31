#pragma once
#include "Object.h"
#include "WindowModule.h"
#include "GraphicsModule.h"
#include "ThreadManager.h"
#include "JobManager.h"
#include "GameThread.h"
#include "GraphicsThread.h"
#include "ScenesModule.h"
#include "RendererModule.h"
#include "MainThread.h"
#include "ResourceLoaderModule.h"
#include "TimerModule.h"
#include "ProfilerModule.h"
#include "ResourceType.h"

namespace Glory
{
	struct EngineCreateInfo
	{
		WindowModule* pWindowModule;
		ScenesModule* pScenesModule;
		RendererModule* pRenderModule;
		GraphicsModule* pGraphicsModule;

		uint32_t OptionalModuleCount;
		Module** pOptionalModules;
	};

	/// <summary>
	/// This class describes the engine a GloryGame object will run on.
	/// It holds the required modules used to run the game, as well as optional modules.
	/// </summary>
	class Engine : public Object
	{
	public:
		static Engine* CreateEngine(const EngineCreateInfo& createInfo);

		WindowModule* GetWindowModule() const;
		ScenesModule* GetScenesModule() const;
		RendererModule* GetRendererModule() const;
		GraphicsModule* GetGraphicsModule() const;
		TimerModule* GetTimerModule() const;
		ProfilerModule* GetProfilerModule() const;

		template<class T>
		T* GetModule()
		{
			Module* pModule = GetModule(typeid(T));
			return (T*)pModule;
		}

		template<class T>
		LoaderModule* GetLoaderModule()
		{
			LoaderModule* pModule = GetLoaderModule(typeid(T));
			return pModule;
		}

		LoaderModule* GetLoaderModule(const std::string& extension);

		LoaderModule* GetLoaderModule(const std::type_info& resourceType);
		LoaderModule* GetLoaderModule(size_t typeHash);

		Module* GetModule(const std::type_info& type);

		GameThread* GetGameThread() const;
		GraphicsThread* GetGraphicsThread() const;

		void StartThreads();
		void Update();

	private:
		Engine(const EngineCreateInfo& createInfo);
		virtual ~Engine();

	private:
		void Initialize();

	private:
		void GameThreadFrameStart();
		void GameThreadFrameEnd();
		void GraphicsThreadFrameStart();
		void GraphicsThreadFrameEnd();

	private:
		friend class Game;
		friend class GameThread;
		friend class GraphicsThread;

		// Required modules
		WindowModule* m_pWindowModule;
		ScenesModule* m_pScenesModule;
		RendererModule* m_pRenderModule;
		GraphicsModule* m_pGraphicsModule;
		TimerModule* m_pTimerModule;
		ProfilerModule* m_pProfilerModule;

		// Optional modules
		std::vector<Module*> m_pOptionalModules;

		// All modules
		std::vector<Module*> m_pAllModules;

		std::vector<Module*> m_pPriorityInitializationModules;

		std::vector<LoaderModule*> m_pLoaderModules;
		std::map<std::type_index, size_t> m_TypeToLoader;
		std::map<size_t, size_t> m_TypeHashToLoader;

		// Threading
		ThreadManager* m_pThreadManager;
		Jobs::JobManager* m_pJobManager;

		// Main threads
		MainThread* m_pMainThread;
		GameThread* m_pGameThread;
		GraphicsThread* m_pGraphicsThread;
	};
}
