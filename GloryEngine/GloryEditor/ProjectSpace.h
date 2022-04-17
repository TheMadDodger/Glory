#pragma once
#include <string>
#include <mutex>
#include <functional>

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
		static void OpenProject(const std::string& path);
		static void CloseProject();
		static ProjectSpace* GetOpenProject();
		static bool ProjectExists(const std::string& path);
		static bool ProjectExists(const std::string& path, const std::string& name);
		static std::string NewProject(const std::string& path, const std::string& name);
		void CreateFolder(const std::string& name);
		void ImportModuleAssets(bool overwrite);

		std::string Name();
		std::string RootPath();
		std::string ProjectPath();
		std::string CachePath();

		static void RegisterCallback(const ProjectCallback& callbackType, std::function<void(ProjectSpace*)> callback);

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
		std::string m_ProjectName;
	};
}
