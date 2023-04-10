#pragma once
#include <string>
#include <mutex>
#include <functional>
#include <string>

#include "GloryEditor.h"
#include "JSONRef.h"

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

		GLORY_EDITOR_API const std::string& Name() const;
		GLORY_EDITOR_API const std::string& RootPath() const;
		GLORY_EDITOR_API const std::string& ModuleSettingsPath() const;
		GLORY_EDITOR_API const std::string& ProjectPath() const;
		GLORY_EDITOR_API const std::string& CachePath() const;
		GLORY_EDITOR_API const std::string& LibraryPath() const;
		GLORY_EDITOR_API const std::string& SettingsPath() const;

		static GLORY_EDITOR_API void RegisterCallback(const ProjectCallback& callbackType, std::function<void(ProjectSpace*)> callback);
		static GLORY_EDITOR_API void SetAssetDirty(const char* key, bool dirty = true);
		static GLORY_EDITOR_API bool HasUnsavedChanges();

		JSONFileRef& ProjectFile();

	private:
		ProjectSpace(const std::string& path);
		virtual ~ProjectSpace();

		void Open();
		void Close();

	private:
		static ProjectSpace* m_pCurrentProject;
		static std::unordered_map<ProjectCallback, std::vector<std::function<void(ProjectSpace*)>>> m_ProjectCallbacks;
		static std::mutex m_ProjectLock;
		std::string m_ProjectFilePath;
		std::string m_ProjectRootPath;
		std::string m_CachePath;
		std::string m_LibraryPath;
		std::string m_ModuleSettingsPath;
		std::string m_SettingsPath;
		std::string m_ProjectName;
		std::vector<std::string> m_DirtyKeys;
		JSONFileRef m_ProjectFile;
	};
}
