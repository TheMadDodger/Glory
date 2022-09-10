#pragma once
#include "ModuleManager.h"
#include <Versioning.h>

namespace Glory::EditorLauncher
{
	struct Project
	{
		std::string Name;
		std::string Path;
		Glory::Version Version;
		mutable Glory::Version SelectedVersion;
		long long LastEdit;
	};

	struct EngineSettings
	{
		int RenderModule;
		int GraphicsModule;
		int WindowModule;
		int ScenesModule;
		std::vector<int> OptionalModules;
	};

	struct ProjectCreateSettings
	{
		std::string Name;
		std::string Path;

		EngineSettings EngineSettings;
		Version EditorVersion;
	};

	class ProjectManager
	{
	public:
		static void OpenProject(size_t index);
		static void CreateProject(const ProjectCreateSettings& createSettings);
		static void RemoveProjectAt(size_t index);
		static void AddProject(const std::string& path);

		static size_t ProjectCount();
		static const Project* GetProject(size_t index);

		static void Load();
		static void Save();

	private:
		static void WriteModule(const ModuleType& moduleType, int index, YAML::Emitter& emitter);
		static void WriteEngineConfig(const ProjectCreateSettings& createSettings);
		static void WriteProjectFile(const ProjectCreateSettings& createSettings);

	private:
		ProjectManager();
		virtual ~ProjectManager();

	private:
		static std::vector<Project> m_Projects;
	};
}
