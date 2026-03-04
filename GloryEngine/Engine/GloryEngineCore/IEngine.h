#pragma once
#include <typeinfo>
#include <string>
#include <filesystem>

namespace Glory
{
	class Module;
	class LoaderModule;
	class SceneManager;
	class IModuleLoopHandler;
	class ThreadManager;
	class Console;
	class Debug;
	class GameTime;
	class CameraManager;
	class AssetDatabase;
	class AssetManager;
	class ResourceTypes;
	class LayerManager;
	class MaterialManager;
	class PipelineManager;
	class ObjectManager;
	class EngineProfiler;
	class GraphicsDevice;
	class Renderer;

	struct WindowCreateInfo;
	struct Version;
	struct UUIDRemapper;
	struct Version;

	namespace Jobs
	{
		class JobManager;
	}

	namespace Utils::Reflect
	{
		class Reflect;
	}

	class IEngine
	{
	public: /* Template helpers */
		template<class T>
		T* GetMainModule() const
		{
			Module* pModule = GetMainModule(typeid(T));
			return pModule ? (T*)pModule : nullptr;
		}

		template<class T>
		T* GetOptionalModule() const
		{
			Module* pModule = GetOptionalModule(typeid(T));
			return pModule ? (T*)pModule : nullptr;
		}

		template<class T>
		T* GetModule() const
		{
			Module* pModule = GetModule(typeid(T));
			return pModule ? (T*)pModule : nullptr;
		}

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

	public:
		virtual const Version& EngineVersion() const = 0;

		virtual SceneManager* GetSceneManager() = 0;

		virtual void AddMainModule(Module* pModule, bool initialize = false) = 0;
		virtual void AddOptionalModule(Module* pModule, bool initialize = false) = 0;
		virtual void AddInternalModule(Module* pModule, bool initialize = false) = 0;
		virtual void AddLoaderModule(LoaderModule* pModule, bool initialize = false) = 0;

		virtual Module* GetMainModule(const std::type_info& type) const = 0;
		virtual Module* GetMainModule(const std::string& name) const = 0;
		virtual Module* GetOptionalModule(const std::type_info& type) const = 0;
		virtual Module* GetOptionalModule(const std::string& name) const = 0;
		virtual Module* GetModule(const std::type_info& type) const = 0;
		virtual Module* GetModule(const std::string& name) const = 0;
		virtual Module* GetInternalModule(const std::type_info& type) const = 0;

		virtual LoaderModule* GetLoaderModule(const std::string& extension) = 0;
		virtual LoaderModule* GetLoaderModule(const std::type_info& resourceType) = 0;
		virtual LoaderModule* GetLoaderModule(uint32_t typeHash) = 0;

		virtual void UpdateSceneManager() = 0;
		virtual void DrawSceneManager() = 0;
		virtual void ModulesLoop(IModuleLoopHandler* pLoopHandler = nullptr) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void CallModuleUpdate(Module* pModule) const = 0;
		virtual void CallModuleDraw(Module* pModule) const = 0;

		virtual const size_t InternalModulesCount() const = 0;
		virtual const size_t ModulesCount() const = 0;
		virtual Module* GetModule(size_t index) const = 0;

		virtual void LoadModuleSettings(const std::filesystem::path& overrideRootPath = "") = 0;

		virtual void Update() = 0;
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Draw() = 0;

		virtual ThreadManager& Threads() = 0;
		virtual Jobs::JobManager& Jobs() = 0;
		virtual Console& GetConsole() = 0;
		virtual Debug& GetDebug() = 0;
		virtual GameTime& Time() = 0;
		virtual CameraManager& GetCameraManager() = 0;
		virtual AssetDatabase& GetAssetDatabase() = 0;
		virtual AssetManager& GetAssetManager() = 0;
		virtual ResourceTypes& GetResourceTypes() = 0;
		virtual LayerManager& GetLayerManager() = 0;
		virtual MaterialManager& GetMaterialManager() = 0;
		virtual PipelineManager& GetPipelineManager() = 0;
		virtual Utils::Reflect::Reflect& Reflection() = 0;
		virtual ObjectManager& GetObjectManager() = 0;
		virtual EngineProfiler& Profiler() = 0;

		virtual void SetAssetManager(AssetManager* pManager) = 0;
		virtual void SetSceneManager(SceneManager* pManager) = 0;
		virtual void SetMaterialManager(MaterialManager* pManager) = 0;
		virtual void SetPipelineManager(PipelineManager* pManager) = 0;

		virtual void AddUserContext(uint32_t hash, void* pUserContext) = 0;
		virtual void* GetUserContext(uint32_t hash) = 0;

		virtual void RequestQuit() = 0;
		virtual void CancelQuit() = 0;
		virtual bool WantsToQuit() const = 0;

		virtual void AddData(const std::filesystem::path& path, const std::string& name, std::vector<char>&& data) = 0;
		virtual void ProcessData() = 0;
		virtual bool HasData(const std::string& name) const = 0;
		virtual const std::filesystem::path& DataPath(const std::string& name) const = 0;
		virtual std::vector<char>& GetData(const std::string& name) = 0;

		virtual void SetRootPath(const std::filesystem::path& path) = 0;
		virtual const std::filesystem::path& RootPath() const = 0;

		virtual void SetApplicationVersion(uint32_t major, uint32_t minor, uint32_t subMinor, uint32_t rc = 0) = 0;
		virtual const Version& GetApplicationVersion() const = 0;

		virtual void SetOrganizationAndAppName(std::string&& organization, std::string&& appName) = 0;
		virtual std::string_view Organization() const = 0;
		virtual std::string_view AppName() const = 0;

		virtual void SetMainWindowInfo(WindowCreateInfo&& info) = 0;
		virtual WindowCreateInfo& MainWindowInfo() = 0;

		virtual void AddGraphicsDevice(GraphicsDevice* pGraphicsDevice) = 0;
		virtual GraphicsDevice* ActiveGraphicsDevice() = 0;

		virtual void AddMainRenderer(Renderer* pRenderer) = 0;
		virtual Renderer* ActiveRenderer() = 0;

		virtual void Load() = 0;

		virtual UUIDRemapper& GetUUIDRemapper() = 0;
	};
}