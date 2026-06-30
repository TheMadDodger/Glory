#pragma once
#include "ResourceType.h"
#include "WindowData.h"

#include "UUIDRemapper.h"

#include <engine_visibility.h>

#include <Object.h>
#include <IEngine.h>
#include <Version.h>

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
	class Resources;
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

	class Resources;
	class ResourceLoader;

	namespace Jobs
	{
		class JobManager;
	}

	struct EngineCreateInfo
	{
		Debug* m_pDebug;
		Console* m_pConsole;
		Resources* pAssetManager = nullptr;
		SceneManager* pSceneManager = nullptr;
		MaterialManager* pMaterialManager = nullptr;
		PipelineManager* pPipelineManager = nullptr;
		ResourceLoader* pResourceLoader = nullptr;

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
	class GloryEngine final : public IEngine
	{
	public:
		GLORY_ENGINE_API GloryEngine(const EngineCreateInfo& createInfo);
		GLORY_ENGINE_API virtual ~GloryEngine();

	public:
		GLORY_ENGINE_API const Version& EngineVersion() const override;

		GLORY_ENGINE_API SceneManager* GetSceneManager() override;

		GLORY_ENGINE_API void AddMainModule(Module* pModule, bool initialize = false) override;
		GLORY_ENGINE_API void AddOptionalModule(Module* pModule, bool initialize = false) override;
		GLORY_ENGINE_API void AddInternalModule(Module* pModule, bool initialize = false) override;
		GLORY_ENGINE_API void AddLoaderModule(LoaderModule* pModule, bool initialize = false) override;

		GLORY_ENGINE_API Module* GetMainModule(const std::type_info& type) const override;
		GLORY_ENGINE_API Module* GetMainModule(const std::string& name) const override;
		GLORY_ENGINE_API Module* GetOptionalModule(const std::type_info& type) const override;
		GLORY_ENGINE_API Module* GetOptionalModule(const std::string& name) const override;
		GLORY_ENGINE_API Module* GetModule(const std::type_info& type) const override;
		GLORY_ENGINE_API Module* GetModule(const std::string& name) const override;
		GLORY_ENGINE_API Module* GetInternalModule(const std::type_info& type) const override;
		GLORY_ENGINE_API LoaderModule* GetLoaderModule(const std::string& extension) override;
		GLORY_ENGINE_API LoaderModule* GetLoaderModule(const std::type_info& resourceType) override;
		GLORY_ENGINE_API LoaderModule* GetLoaderModule(uint32_t typeHash) override;

		GLORY_ENGINE_API void UpdateSceneManager(float dt) override;
		GLORY_ENGINE_API void DrawSceneManager() override;
		GLORY_ENGINE_API void ModulesLoop(IModuleLoopHandler* pLoopHandler = nullptr) override;
		GLORY_ENGINE_API void BeginFrame() override;
		GLORY_ENGINE_API void EndFrame() override;

		GLORY_ENGINE_API void CallModuleUpdate(Module* pModule) const override;
		GLORY_ENGINE_API void CallModuleDraw(Module* pModule) const override;

		GLORY_ENGINE_API const size_t InternalModulesCount() const override;
		GLORY_ENGINE_API const size_t ModulesCount() const override;
		GLORY_ENGINE_API Module* GetModule(size_t index) const override;

		GLORY_ENGINE_API void LoadLegacyModuleSettings(const std::filesystem::path& overrideRootPath = "") override;
		GLORY_ENGINE_API void LoadModuleSettings(const std::filesystem::path& settingsPath) override;

		GLORY_ENGINE_API void RegisterTypes() override;
		GLORY_ENGINE_API void Update() override;
		GLORY_ENGINE_API void Initialize() override;
		GLORY_ENGINE_API void Cleanup() override;
		GLORY_ENGINE_API void Draw() override;

		GLORY_ENGINE_API Resources& GetResources() override;
		GLORY_ENGINE_API ThreadManager& Threads() override;
		GLORY_ENGINE_API Jobs::JobManager& Jobs() override;
		GLORY_ENGINE_API Console& GetConsole() override;
		GLORY_ENGINE_API Debug& GetDebug() override;
		GLORY_ENGINE_API GameTime& Time() override;
		GLORY_ENGINE_API CameraManager& GetCameraManager() override;
		GLORY_ENGINE_API AssetDatabase& GetAssetDatabase() override;
		GLORY_ENGINE_API ResourceTypes& GetResourceTypes() override;
		GLORY_ENGINE_API LayerManager& GetLayerManager() override;
		GLORY_ENGINE_API MaterialManager& GetMaterialManager() override;
		GLORY_ENGINE_API PipelineManager& GetPipelineManager() override;
		GLORY_ENGINE_API ResourceLoader& GetResourceLoader() override;
		GLORY_ENGINE_API Utils::Reflect::Reflect& Reflection() override;
		GLORY_ENGINE_API ObjectManager& GetObjectManager() override;
		GLORY_ENGINE_API EngineProfiler& Profiler() override;

		GLORY_ENGINE_API void SetSceneManager(SceneManager* pManager) override;
		GLORY_ENGINE_API void SetMaterialManager(MaterialManager* pManager) override;
		GLORY_ENGINE_API void SetPipelineManager(PipelineManager* pManager) override;
		GLORY_ENGINE_API void SetResourceLoader(ResourceLoader* pLoader) override;

		GLORY_ENGINE_API void AddUserContext(uint32_t hash, void* pUserContext) override;
		GLORY_ENGINE_API void* GetUserContext(uint32_t hash) override;

		GLORY_ENGINE_API void RequestQuit() override;
		GLORY_ENGINE_API void CancelQuit() override;
		GLORY_ENGINE_API bool WantsToQuit() const override;

		GLORY_ENGINE_API void AddData(const std::filesystem::path& path, const std::string& name, std::vector<char>&& data) override;
		GLORY_ENGINE_API void ProcessData() override;
		GLORY_ENGINE_API bool HasData(const std::string& name) const override;
		GLORY_ENGINE_API const std::filesystem::path& DataPath(const std::string& name) const override;
		GLORY_ENGINE_API std::vector<char>& GetData(const std::string& name) override;

		GLORY_ENGINE_API void SetRootPath(const std::filesystem::path& path) override;
		GLORY_ENGINE_API const std::filesystem::path& RootPath() const override;

		GLORY_ENGINE_API void SetApplicationVersion(uint32_t major, uint32_t minor, uint32_t subMinor, uint32_t rc=0) override;
		GLORY_ENGINE_API const Version& GetApplicationVersion() const override;

		GLORY_ENGINE_API void SetOrganizationAndAppName(std::string&& organization, std::string&& appName) override;
		GLORY_ENGINE_API std::string_view Organization() const override;
		GLORY_ENGINE_API std::string_view AppName() const override;

		GLORY_ENGINE_API void SetMainWindowInfo(WindowCreateInfo&& info) override;
		GLORY_ENGINE_API WindowCreateInfo& MainWindowInfo() override;

		GLORY_ENGINE_API void AddGraphicsDevice(GraphicsDevice* pGraphicsDevice) override;
		GLORY_ENGINE_API GraphicsDevice* ActiveGraphicsDevice() override;

		GLORY_ENGINE_API void AddMainRenderer(Renderer* pRenderer) override;
		GLORY_ENGINE_API Renderer* ActiveRenderer() override;

		GLORY_ENGINE_API void Load() override;

		GLORY_ENGINE_API UUIDRemapper& GetUUIDRemapper() override;

		template<class T>
		inline void AddUserContext(T* pUserContext)
		{
			AddUserContext(ResourceType::GetHash(typeid(T)), (void*)pUserContext);
		}

		template<class T>
		inline T* GetUserContext()
		{
			return (T*)GetUserContext(ResourceType::GetHash(typeid(T)));
		}

	private:
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
		Resources* m_pAssetsManager;
		MaterialManager* m_pMaterialManager;
		PipelineManager* m_pPipelineManager;
		ResourceLoader* m_pResourceLoader;

		bool m_Quit{ false };
		bool m_Initialized{ false };

		/* Owned objects */
		std::unique_ptr<Resources> m_Resources;
		std::unique_ptr<GameTime> m_Time;
		std::unique_ptr<CameraManager> m_CameraManager;
		std::unique_ptr<AssetDatabase> m_AssetDatabase;
		std::unique_ptr<ResourceTypes> m_ResourceTypes;
		std::unique_ptr<LayerManager> m_LayerManager;
		std::unique_ptr<Utils::Reflect::Reflect> m_Reflection;
		std::unique_ptr<ObjectManager> m_ObjectManager;
		std::unique_ptr<EngineProfiler> m_Profiler;
		std::map<size_t, void*> m_pUserContexts;
		std::map<std::string, std::vector<char>> m_Datas;
		std::map<std::string, std::filesystem::path> m_DataPaths;

		std::filesystem::path m_RootPath;

		Version m_ApplicationVersion;
		std::string m_Organization;
		std::string m_AppName;

		UUIDRemapper m_UUIDRemapper;
	};
}
