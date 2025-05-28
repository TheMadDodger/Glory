#pragma once
#include "GloryEditor.h"

#include <UUID.h>
#include <functional>
#include <atomic>
#include <string>

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class GScene;
	class Engine;
	class AssetArchive;

	namespace Utils
	{
		struct YAMLFileRef;
	}
}

namespace Glory::Editor
{
	template<typename Arg>
	struct Dispatcher;

	struct EmptyEvent {};

	using EmptyDispatcher = Dispatcher<EmptyEvent>;

	struct PackageTaskState
	{
		void Reset()
		{
			m_ProcessedSubTasks = 0;
			m_TotalSubTasks = 0;
		}

		std::atomic<size_t> m_ProcessedSubTasks;
		std::atomic<size_t> m_TotalSubTasks;
		std::atomic<std::string_view> m_SubTaskName;
	};

	struct PackageTask
	{
		std::string m_TaskID;
		std::string m_TaskName;
		std::function<bool(Glory::Engine*, const std::filesystem::path&, PackageTaskState&)> m_Callback;
		size_t m_TotalSubTasks{ 0 };
	};

	GLORY_EDITOR_API void StartPackage(Glory::Engine* pEngine);
	GLORY_EDITOR_API void CancelPackage();
	GLORY_EDITOR_API void AddPackagingTask(PackageTask&& task, const std::string& before);
	GLORY_EDITOR_API void AddPackagingTaskAfter(PackageTask&& task, const std::string& after);
	GLORY_EDITOR_API bool PackageState(size_t& currentIndex, size_t& count, std::string_view& name, size_t& subIndex, size_t& subCount, std::string_view& subName);
	GLORY_EDITOR_API bool IsPackagingBusy();
	GLORY_EDITOR_API bool PackageFailed();
	GLORY_EDITOR_API bool IsAssetPackaged(UUID assetID);
	GLORY_EDITOR_API void Package(Glory::Engine* pEngine);
	GLORY_EDITOR_API void ScanSceneFileForAssets(Glory::Engine* pEngine, Utils::YAMLFileRef& file, std::vector<UUID>& assets);
	GLORY_EDITOR_API void ScanSceneFileForAssets(Glory::Engine* pEngine, Utils::YAMLFileRef& file, std::vector<UUID>& assets);
	GLORY_EDITOR_API void PackageScene(GScene* pScene, const std::filesystem::path& path);
	GLORY_EDITOR_API size_t NumScenesToPackage();
	GLORY_EDITOR_API UUID SceneToPackage(size_t index);
	GLORY_EDITOR_API GScene* LoadedSceneToPackage(size_t index);
	GLORY_EDITOR_API EmptyDispatcher& GatherPackageTasksEvents();
	GLORY_EDITOR_API EmptyDispatcher& PackagingStartedEvent();
	GLORY_EDITOR_API EmptyDispatcher& PackagingEndedEvent();
}