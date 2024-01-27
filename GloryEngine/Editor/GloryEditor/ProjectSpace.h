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

		Count
	};

	template<typename Arg>
	struct Dispatcher;

	class ProjectSpace
	{
	public:
		GLORY_EDITOR_API static void OpenProject(const std::string& path);
		GLORY_EDITOR_API static void CloseProject();
		GLORY_EDITOR_API static ProjectSpace* GetOpenProject();
		GLORY_EDITOR_API static bool ProjectExists(const std::string& path);
		GLORY_EDITOR_API static bool ProjectExists(const std::string& path, const std::string& name);
		GLORY_EDITOR_API static std::string NewProject(const std::string& path, const std::string& name);
		GLORY_EDITOR_API static void Save();

		GLORY_EDITOR_API void CreateFolder(const std::string& name);
		GLORY_EDITOR_API void SaveProject();

		GLORY_EDITOR_API const std::string& Name() const;
		GLORY_EDITOR_API const std::string& RootPath() const;
		GLORY_EDITOR_API const std::string& ModuleSettingsPath() const;
		GLORY_EDITOR_API const std::string& ProjectPath() const;
		GLORY_EDITOR_API const std::string& CachePath() const;
		GLORY_EDITOR_API const std::string& LibraryPath() const;
		GLORY_EDITOR_API const std::string& SettingsPath() const;

		GLORY_EDITOR_API static UUID RegisterCallback(const ProjectCallback& callbackType, std::function<void(ProjectSpace*)> callback);
		GLORY_EDITOR_API static void RemoveCallback(const ProjectCallback& callbackType, UUID& id);
		GLORY_EDITOR_API static void SetAssetDirty(const char* key, bool dirty = true);
		GLORY_EDITOR_API static bool HasUnsavedChanges();

		GLORY_EDITOR_API JSONFileRef& ProjectFile();

	private:
		ProjectSpace(const std::string& path);
		virtual ~ProjectSpace();

		void Open();
		void Close();

	private:
		static ProjectSpace* m_pCurrentProject;
		static std::recursive_mutex m_ProjectLock;
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
