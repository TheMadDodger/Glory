#pragma once
#include "Object.h"
#include "ResourceType.h"
#include "WindowData.h"

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
	class ThreadManager;
	class LoaderModule;
	class SceneManager;
	class PropertySerializer;

	class AssetDatabase;
	class AssetManager;
	class MaterialManager;
	class PipelineManager;
	class GraphicsDevice;
	class Renderer;

	class Debug;
	class Console;
	class Serializers;
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
		AssetManager* pAssetManager = nullptr;
		SceneManager* pSceneManager = nullptr;
		MaterialManager* pMaterialManager = nullptr;
		PipelineManager* pPipelineManager = nullptr;

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

		void UpdateSceneManager();
		void DrawSceneManager();
		void ModulesLoop(IModuleLoopHandler* pLoopHandler = nullptr);
		void BeginFrame();
		void EndFrame();

		void CallModuleUpdate(Module* pModule);
		void CallModuleDraw(Module* pModule);

		const size_t InternalModulesCount() const;
		const size_t ModulesCount() const;
		Module* GetModule(size_t index) const;

		void LoadModuleSettings(const std::filesystem::path& overrideRootPath = "");

		void Update();
		void Initialize();
		void Cleanup();
		void Draw();

		ThreadManager& Threads();
		Jobs::JobManager& Jobs();
		Console& GetConsole();
		Debug& GetDebug();
		GameTime& Time();
		CameraManager& GetCameraManager();
		AssetDatabase& GetAssetDatabase();
		AssetManager& GetAssetManager();
		ResourceTypes& GetResourceTypes();
		Serializers& GetSerializers();
		LayerManager& GetLayerManager();
		MaterialManager& GetMaterialManager();
		PipelineManager& GetPipelineManager();
		Utils::Reflect::Reflect& Reflection();
		ObjectManager& GetObjectManager();
		EngineProfiler& Profiler();

		void SetAssetManager(AssetManager* pManager);
		void SetSceneManager(SceneManager* pManager);
		void SetMaterialManager(MaterialManager* pManager);
		void SetPipelineManager(PipelineManager* pManager);

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

		void AddData(const std::filesystem::path& path, const std::string& name, std::vector<char>&& data);
		void ProcessData();
		bool HasData(const std::string& name) const;
		const std::filesystem::path& DataPath(const std::string& name) const;
		std::vector<char>& GetData(const std::string& name);

		void SetRootPath(const std::filesystem::path& path);
		const std::filesystem::path& RootPath() const;

		void SetApplicationVersion(uint32_t major, uint32_t minor, uint32_t subMinor, uint32_t rc=0);
		const Version& GetApplicationVersion() const;

		void SetOrganizationAndAppName(std::string&& organization, std::string&& appName);
		std::string_view Organization() const;
		std::string_view AppName() const;

		void SetMainWindowInfo(WindowCreateInfo&& info);
		WindowCreateInfo& MainWindowInfo();

		void AddGraphicsDevice(GraphicsDevice* pGraphicsDevice);
		GraphicsDevice* ActiveGraphicsDevice();

		void AddMainRenderer(Renderer* pRenderer);
		Renderer* ActiveRenderer();

		void Load();

	private:
		void RegisterStandardSerializers();
		void RegisterBasicTypes();

	private:
		friend class GameThread;

		/* Original create info*/
		const EngineCreateInfo m_CreateInfo;

		WindowCreateInfo m_MainWindowInfo;

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

		size_t m_ActiveGraphicsDevice;
		std::vector<GraphicsDevice*> m_pGraphicsDevices;
		size_t m_ActiveRenderer;
		std::vector<Renderer*> m_pRenderers;
		std::vector<Renderer*> m_pSecondaryRenderers;

		/* Threading */
		std::unique_ptr<ThreadManager> m_ThreadManager;
		std::unique_ptr<Jobs::JobManager> m_JobManager;

		/* External objects */
		Console* m_Console;
		Debug* m_Debug;
		AssetManager* m_pAssetsManager;
		MaterialManager* m_pMaterialManager;
		PipelineManager* m_pPipelineManager;

		bool m_Quit{ false };
		bool m_Initialized{ false };

		/* Owned objects */
		std::unique_ptr<GameTime> m_Time;
		std::unique_ptr<CameraManager> m_CameraManager;
		std::unique_ptr<AssetDatabase> m_AssetDatabase;
		std::unique_ptr<ResourceTypes> m_ResourceTypes;
		std::unique_ptr<Serializers> m_Serializers;
		std::unique_ptr<LayerManager> m_LayerManager;
		std::unique_ptr<Utils::Reflect::Reflect> m_Reflection;
		std::unique_ptr<ObjectManager> m_ObjectManager;
		std::unique_ptr<EngineProfiler> m_Profiler;
		std::map<size_t, void*> m_pUserContexts;
		std::vector<PropertySerializer*> m_pRegisteredPropertySerializers;
		std::map<std::string, std::vector<char>> m_Datas;
		std::map<std::string, std::filesystem::path> m_DataPaths;

		std::filesystem::path m_RootPath;

		Version m_ApplicationVersion;
		std::string m_Organization;
		std::string m_AppName;
	};
}
