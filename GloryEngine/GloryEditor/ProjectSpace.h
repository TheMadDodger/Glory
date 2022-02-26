#pragma once
#include <string>

namespace Glory::Editor
{
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

		std::string Name();
		std::string RootPath();
		std::string ProjectPath();
		std::string CachePath();

	private:
		ProjectSpace(const std::string& path);
		virtual ~ProjectSpace();

		void Open();
		void Close();

	private:
		static ProjectSpace* m_pCurrentProject;
		std::string m_ProjectFilePath;
		std::string m_ProjectRootPath;
		std::string m_CachePath;
		std::string m_ProjectName;
	};
}
