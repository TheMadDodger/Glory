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

		template<class T>
		T* GetModule()
		{
			Module* pModule = GetModule(typeid(T));
			return (T*)pModule;
		}

		Module* GetModule(const std::type_info& type);

		GameThread* GetGameThread() const;
		GraphicsThread* GetGraphicsThread() const;

	private:
		Engine(const EngineCreateInfo& createInfo);
		virtual ~Engine();

	private:
		void Initialize();
		void Update();

	private:
		friend class Game;

		// Required modules
		WindowModule* m_pWindowModule;
		ScenesModule* m_pScenesModule;
		RendererModule* m_pRenderModule;
		GraphicsModule* m_pGraphicsModule;

		// Optional modules
		std::vector<Module*> m_pOptionalModules;

		// All modules
		std::vector<Module*> m_pAllModules;

		std::vector<Module*> m_pPriorityInitializationModules;

		// Threading
		ThreadManager* m_pThreadManager;
		Jobs::JobManager* m_pJobManager;

		// Main threads
		MainThread* m_pMainThread;
		GameThread* m_pGameThread;
		GraphicsThread* m_pGraphicsThread;
	};
}
