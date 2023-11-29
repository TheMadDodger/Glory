#pragma once
#include "Object.h"
#include "ResourceType.h"
#include "GloryCore.h"

#include <filesystem>

namespace Glory
{
	class Module;
	class IModuleLoopHandler;
	class ScriptingExtender;
	class GraphicsThread;
	class ThreadManager;
	class ScriptingModule;
	class LoaderModule;
	class SceneManager;

	namespace Jobs
	{
		class JobManager;
	}

	struct EngineCreateInfo
	{
		uint32_t MainModuleCount;
		/* Order should be: 
		 * - WindowModule
		 * - GraphicsModule
		 * - RenderModule
		 * - InputModule
		 */
		Module** pMainModules;

		uint32_t ScriptingModulesCount;
		ScriptingModule** pScriptingModules;

		uint32_t OptionalModuleCount;
		Module** pOptionalModules;
	};

	/// <summary>
	/// This class describes the engine a GloryGame object will run on.
	/// It holds the required modules used to run the game, as well as optional modules.
	/// </summary>
	class Engine
	{
	public:
		static Engine* CreateEngine(const EngineCreateInfo& createInfo);

		SceneManager* GetSceneManager();

		void AddMainModule(Module* pModule, bool initialize = false);
		void AddOptionalModule(Module* pModule, bool initialize = false);
		void AddInternalModule(Module* pModule, bool initialize = false);

		ScriptingExtender* GetScriptingExtender() const;

		Module* GetMainModule(const std::type_info& type) const;
		Module* GetMainModule(const std::string& name) const;

		template<class T>
		T* GetMainModule() const
		{
			Module* pModule = GetMainModule(typeid(T));
			return pModule ? (T*)pModule : nullptr;
		}

		Module* GetOptionalModule(const std::type_info& type) const;
		Module* GetOptionalModule(const std::string& name) const;

		template<class T>
		T* GetOptionalModule() const
		{
			Module* pModule = GetOptionalModule(typeid(T));
			return pModule ? (T*)pModule : nullptr;
		}

		Module* GetModule(const std::type_info& type) const;
		Module* GetModule(const std::string& name) const;

		template<class T>
		T* GetModule() const
		{
			Module* pModule = GetModule(typeid(T));
			return pModule ? (T*)pModule : nullptr;
		}

		Module* GetInternalModule(const std::type_info& type) const;

		template<class T>
		T* GetInternalModule() const
		{
			Module* pModule = GetInternalModule(typeid(T));
			return pModule ? (T*)pModule : nullptr;
		}

		template<class T>
		T* GetScriptingModule()
		{
			for (size_t i = 0; i < m_pScriptingModules.size(); i++)
			{
				T* pScriptingModule = dynamic_cast<T*>(m_pScriptingModules[i]);
				if (pScriptingModule) return pScriptingModule;
			}

			return nullptr;
		}

		template<class T>
		LoaderModule* GetLoaderModule()
		{
			LoaderModule* pModule = GetLoaderModule(typeid(T));
			return pModule;
		}

		LoaderModule* GetLoaderModule(const std::string& extension);

		LoaderModule* GetLoaderModule(const std::type_info& resourceType);
		LoaderModule* GetLoaderModule(uint32_t typeHash);

		GraphicsThread* GetGraphicsThread() const;

		void StartThreads();
		void UpdateSceneManager();
		void DrawSceneManager();
		void ModulesLoop(IModuleLoopHandler* pLoopHandler = nullptr);
		void GameThreadFrameStart();
		void GameThreadFrameEnd();

		void CallModuleUpdate(Module* pModule);
		void CallModuleDraw(Module* pModule);

		const size_t ModulesCount() const;
		Module* GetModule(size_t index) const;

		void LoadModuleSettings(const std::filesystem::path& overrideRootPath = "");

	private:
		Engine(const EngineCreateInfo& createInfo);
		virtual ~Engine();

	private:
		void Update();
		void Initialize();

		void RegisterStandardSerializers();
		void RegisterBasicTypes();

	private:
		void GraphicsThreadFrameStart();
		void GraphicsThreadFrameEnd();

	private:
		friend class Game;
		friend class GameThread;
		friend class GraphicsThread;
		friend class ScriptingExtender;

		/* Original create info*/
		const EngineCreateInfo m_CreateInfo;

		/* Scene Manager */
		SceneManager* m_pSceneManager;

		/* Scripting */
		ScriptingExtender* m_pScriptingExtender;
		std::vector<ScriptingModule*> m_pScriptingModules;

		/* Main Modules */
		std::vector<Module*> m_pMainModules;

		/* Optional modules */
		std::vector<Module*> m_pOptionalModules;

		/* All modules */
		std::vector<Module*> m_pAllModules;

		/* Optional modules that have priority over initialization order */
		std::vector<Module*> m_pPriorityInitializationModules;

		/* Internal modules */
		std::vector<Module*> m_pInternalModules;

		std::vector<LoaderModule*> m_pLoaderModules;
		std::map<std::type_index, size_t> m_TypeToLoader;
		std::map<uint32_t, size_t> m_TypeHashToLoader;

		/* Threading */
		ThreadManager* m_pThreadManager;
		Jobs::JobManager* m_pJobManager;

		/* Threads */
		GraphicsThread* m_pGraphicsThread;
	};
}
