#pragma once
#include "Object.h"
#include "ResourceType.h"

#include "UUIDRemapper.h"

#include <memory>
#include <filesystem>
#include <map>
#include <vector>

namespace Glory
{
namespace Utils::Reflect
{
	class Reflect;
}

	class Module;
	class IModuleLoopHandler;
	class GraphicsThread;
	class ThreadManager;
	class LoaderModule;
	class SceneManager;
	class PropertySerializer;

	class AssetDatabase;
	class AssetManager;
	class ShaderManager;
	class MaterialManager;

	class Debug;
	class Console;
	class Serializers;
	class DisplayManager;
	class LayerManager;
	class ObjectManager;
	class CameraManager;
	class GameTime;
	class EngineProfiler;

	namespace Jobs
	{
		class JobManager;
	}

	struct EngineCreateInfo
	{
		Debug* m_pDebug;
		Console* m_pConsole;
		ShaderManager* pShaderManager = nullptr;
		MaterialManager* pMaterialManager = nullptr;

		uint32_t MainModuleCount;
		/* Order should be: 
		 * - WindowModule
		 * - GraphicsModule
		 * - RenderModule
		 * - InputModule
		 */
		Module** pMainModules;

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
		Engine(const EngineCreateInfo& createInfo);
		virtual ~Engine();

	public:
		SceneManager* GetSceneManager();

		void AddMainModule(Module* pModule, bool initialize = false);
		void AddOptionalModule(Module* pModule, bool initialize = false);
		void AddInternalModule(Module* pModule, bool initialize = false);
		void AddLoaderModule(LoaderModule* pModule, bool initialize = false);

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

		void Update();
		void Initialize();
		void Cleanup();

		Console& GetConsole();
		Debug& GetDebug();
		GameTime& Time();
		CameraManager& GetCameraManager();
		AssetDatabase& GetAssetDatabase();
		AssetManager& GetAssetManager();
		ResourceTypes& GetResourceTypes();
		Serializers& GetSerializers();
		DisplayManager& GetDisplayManager();
		LayerManager& GetLayerManager();
		ShaderManager& GetShaderManager();
		MaterialManager& GetMaterialManager();
		Utils::Reflect::Reflect& Reflection();
		ObjectManager& GetObjectManager();
		EngineProfiler& Profiler();

		void SetShaderManager(ShaderManager* pManager);
		void SetMaterialManager(MaterialManager* pManager);

		template<class T>
		void AddUserContext(T* pUserContext)
		{
			AddUserContext(ResourceType::GetHash(typeid(T)), (void*)pUserContext);
		}

		template<class T>
		T* GetUserContext()
		{
			return (T*)GetUserContext(ResourceType::GetHash(typeid(T)));
		}

		void AddUserContext(uint32_t hash, void* pUserContext);
		void* GetUserContext(uint32_t hash);

		void RequestQuit();
		void CancelQuit();
		bool WantsToQuit() const;

		UUIDRemapper m_UUIDRemapper;

	private:
		void RegisterStandardSerializers();
		void RegisterBasicTypes();

	private:
		void GraphicsThreadFrameStart();
		void GraphicsThreadFrameEnd();

	private:
		friend class GameThread;
		friend class GraphicsThread;

		/* Original create info*/
		const EngineCreateInfo m_CreateInfo;

		/* Scene Manager */
		SceneManager* m_pSceneManager;

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

		/* External objects */
		Console* m_Console;
		Debug* m_Debug;
		ShaderManager* m_pShaderManager;
		MaterialManager* m_pMaterialManager;

		bool m_Quit = false;

		/* Owned objects */
		std::unique_ptr<GameTime> m_Time;
		std::unique_ptr<CameraManager> m_CameraManager;
		std::unique_ptr<AssetDatabase> m_AssetDatabase;
		std::unique_ptr<AssetManager> m_AssetManager;
		std::unique_ptr<ResourceTypes> m_ResourceTypes;
		std::unique_ptr<Serializers> m_Serializers;
		std::unique_ptr<DisplayManager> m_DisplayManager;
		std::unique_ptr<LayerManager> m_LayerManager;
		std::unique_ptr<Utils::Reflect::Reflect> m_Reflection;
		std::unique_ptr<ObjectManager> m_ObjectManager;
		std::unique_ptr<EngineProfiler> m_Profiler;
		std::map<size_t, void*> m_pUserContexts;
		std::vector<PropertySerializer*> m_pRegisteredPropertySerializers;
	};
}
