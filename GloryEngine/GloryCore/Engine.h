#pragma once
#include "Object.h"
#include "WindowModule.h"
#include "GraphicsModule.h"
#include "ThreadManager.h"
#include "JobManager.h"

namespace Glory
{
	struct EngineCreateInfo
	{
		WindowModule* pWindowModule;
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

		WindowModule* GetWindowModule();
		GraphicsModule* GetGraphicsModule();

		template<class T>
		T* GetModule()
		{
			Module* pModule = GetModule(typeid(T));
			return (T*)pModule;
		}

		Module* GetModule(const std::type_info& type);

	private:
		Engine(const EngineCreateInfo& createInfo);
		virtual ~Engine();

	private:
		void Initialize();
		void Update();
		void Draw();

	private:
		friend class Game;

		// Required modules
		WindowModule* m_pWindowModule;
		GraphicsModule* m_pGraphicsModule;

		// Optional modules
		std::vector<Module*> m_pOptionalModules;

		// All modules
		std::vector<Module*> m_pAllModules;

		std::vector<Module*> m_pPriorityInitializationModules;

		// Threading
		ThreadManager* m_pThreadManager;
		Jobs::JobManager* m_pJobManager;
	};
}