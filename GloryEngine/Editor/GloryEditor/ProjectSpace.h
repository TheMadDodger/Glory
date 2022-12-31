#pragma once
#include <string>
#include <mutex>
#include <functional>
#include <AssetDatabase.h>
#include "GloryEditor.h"

namespace Glory::Editor
{
	enum ProjectCallback
	{
		OnOpen,
		OnClose,
	};

	class ProjectSpace
	{
	public:
		static GLORY_EDITOR_API void OpenProject(const std::string& path);
		static GLORY_EDITOR_API void CloseProject();
		static GLORY_EDITOR_API ProjectSpace* GetOpenProject();
		static GLORY_EDITOR_API bool ProjectExists(const std::string& path);
		static GLORY_EDITOR_API bool ProjectExists(const std::string& path, const std::string& name);
		static GLORY_EDITOR_API std::string NewProject(const std::string& path, const std::string& name);
		GLORY_EDITOR_API void CreateFolder(const std::string& name);
		static GLORY_EDITOR_API void Save();
		GLORY_EDITOR_API void SaveProject();

		GLORY_EDITOR_API std::string Name();
		GLORY_EDITOR_API std::string RootPath();
		GLORY_EDITOR_API std::string ProjectPath();
		GLORY_EDITOR_API std::string CachePath();
		GLORY_EDITOR_API std::string LibraryPath();
		GLORY_EDITOR_API std::string SettingsPath();

		static GLORY_EDITOR_API void RegisterCallback(const ProjectCallback& callbackType, std::function<void(ProjectSpace*)> callback);

		static GLORY_EDITOR_API void SetAssetDirty(const char* key, bool dirty = true);

		static GLORY_EDITOR_API bool HasUnsavedChanges();

	private:
		ProjectSpace(const std::string& path);
		virtual ~ProjectSpace();

		void Open();
		void Close();

		static void OnAssetDirty(UUID uuid, const ResourceMeta& meta, Resource* pResource);

	private:
		static ProjectSpace* m_pCurrentProject;
		static std::unordered_map<ProjectCallback, std::vector<std::function<void(ProjectSpace*)>>> m_ProjectCallbacks;
		static std::mutex m_ProjectLock;
		std::string m_ProjectFilePath;
		std::string m_ProjectRootPath;
		std::string m_CachePath;
		std::string m_LibraryPath;
		std::string m_SettingsPath;
		std::string m_ProjectName;

		std::vector<std::string> m_DirtyKeys;
	};
}
