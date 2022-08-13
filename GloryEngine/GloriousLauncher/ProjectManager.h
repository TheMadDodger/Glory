#pragma once

namespace Glory::EditorLauncher
{
	struct Project
	{
		std::string Name;
		std::string Path;
		long long LastEdit;
	};

	struct ProjectCreateSettings
	{
		std::string Name;
		std::string Path;
	};

	class ProjectManager
	{
	public:
		static void OpenProject(size_t index);
		static void CreateProject(const ProjectCreateSettings& createSettings);
		static void RemoveProjectAt(size_t index);
		static void AddProject(const std::string& path);

		static size_t ProjectCount();
		static const Project* const GetProject(size_t index);

		static void Load();
		static void Save();

	private:
		ProjectManager();
		virtual ~ProjectManager();

	private:
		static std::vector<Project> m_Projects;
	};
}
