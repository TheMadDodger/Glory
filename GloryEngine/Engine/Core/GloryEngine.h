#pragma once
#include "Object.h"
#include "ResourceType.h"
#include "WindowData.h"

#include "UUIDRemapper.h"

#include <IEngine.h>

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
	class GloryEngine : public IEngine
	{
	public:
		GloryEngine(const EngineCreateInfo& createInfo);
		virtual ~GloryEngine();

	public:
		SceneManager* GetSceneManager() override;

		void AddMainModule(Module* pModule, bool initialize = false) override;
		void AddOptionalModule(Module* pModule, bool initialize = false) override;
		void AddInternalModule(Module* pModule, bool initialize = false) override;
		void AddLoaderModule(LoaderModule* pModule, bool initialize = false) override;

		Module* GetMainModule(const std::type_info& type) const override;
		Module* GetMainModule(const std::string& name) const override;
		Module* GetOptionalModule(const std::type_info& type) const override;
		Module* GetOptionalModule(const std::string& name) const override;
		Module* GetModule(const std::type_info& type) const override;
		Module* GetModule(const std::string& name) const override;
		Module* GetInternalModule(const std::type_info& type) const override;
		LoaderModule* GetLoaderModule(const std::string& extension) override;
		LoaderModule* GetLoaderModule(const std::type_info& resourceType) override;
		LoaderModule* GetLoaderModule(uint32_t typeHash) override;

		void UpdateSceneManager() override;
		void DrawSceneManager() override;
		void ModulesLoop(IModuleLoopHandler* pLoopHandler = nullptr) override;
		void BeginFrame() override;
		void EndFrame() override;

		void CallModuleUpdate(Module* pModule) const override;
		void CallModuleDraw(Module* pModule) const override;

		const size_t InternalModulesCount() const override;
		const size_t ModulesCount() const override;
		Module* GetModule(size_t index) const override;

		void LoadModuleSettings(const std::filesystem::path& overrideRootPath = "") override;

		void Update() override;
		void Initialize() override;
		void Cleanup() override;
		void Draw() override;

		ThreadManager& Threads() override;
		Jobs::JobManager& Jobs() override;
		Console& GetConsole() override;
		Debug& GetDebug() override;
		GameTime& Time() override;
		CameraManager& GetCameraManager() override;
		AssetDatabase& GetAssetDatabase() override;
		AssetManager& GetAssetManager() override;
		ResourceTypes& GetResourceTypes() override;
		Serializers& GetSerializers() override;
		LayerManager& GetLayerManager() override;
		MaterialManager& GetMaterialManager() override;
		PipelineManager& GetPipelineManager() override;
		Utils::Reflect::Reflect& Reflection() override;
		ObjectManager& GetObjectManager() override;
		EngineProfiler& Profiler() override;

		void SetAssetManager(AssetManager* pManager) override;
		void SetSceneManager(SceneManager* pManager) override;
		void SetMaterialManager(MaterialManager* pManager) override;
		void SetPipelineManager(PipelineManager* pManager) override;

		void AddUserContext(uint32_t hash, void* pUserContext) override;
		void* GetUserContext(uint32_t hash) override;

		void RequestQuit() override;
		void CancelQuit() override;
		bool WantsToQuit() const override;

		void AddData(const std::filesystem::path& path, const std::string& name, std::vector<char>&& data) override;
		void ProcessData() override;
		bool HasData(const std::string& name) const override;
		const std::filesystem::path& DataPath(const std::string& name) const override;
		std::vector<char>& GetData(const std::string& name) override;

		void SetRootPath(const std::filesystem::path& path) override;
		const std::filesystem::path& RootPath() const override;

		void SetApplicationVersion(uint32_t major, uint32_t minor, uint32_t subMinor, uint32_t rc=0) override;
		const Version& GetApplicationVersion() const override;

		void SetOrganizationAndAppName(std::string&& organization, std::string&& appName) override;
		std::string_view Organization() const override;
		std::string_view AppName() const override;

		void SetMainWindowInfo(WindowCreateInfo&& info) override;
		WindowCreateInfo& MainWindowInfo() override;

		void AddGraphicsDevice(GraphicsDevice* pGraphicsDevice) override;
		GraphicsDevice* ActiveGraphicsDevice() override;

		void AddMainRenderer(Renderer* pRenderer) override;
		Renderer* ActiveRenderer() override;

		void Load() override;

		UUIDRemapper& GetUUIDRemapper() override;

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

		UUIDRemapper m_UUIDRemapper;
	};
}
