#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include "ProjectSpace.h"

namespace Glory::Editor
{
	ProjectSpace* ProjectSpace::m_pCurrentProject = nullptr;

	void ProjectSpace::OpenProject(const std::string& path)
	{
		CloseProject();
		m_pCurrentProject = new ProjectSpace(path);
		m_pCurrentProject->Open();
	}

	void ProjectSpace::CloseProject()
	{
		if (m_pCurrentProject == nullptr) return;
		m_pCurrentProject->Close();
		delete m_pCurrentProject;
		m_pCurrentProject = nullptr;
	}

	ProjectSpace* ProjectSpace::GetOpenProject()
	{
		return m_pCurrentProject;
	}

	bool ProjectSpace::ProjectExists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	bool ProjectSpace::ProjectExists(const std::string& path, const std::string& name)
	{
		std::filesystem::path projectPath(path);
		projectPath.append(name);
		projectPath.append(name + ".gproj");
		return ProjectExists(projectPath.string());
	}

	std::string ProjectSpace::NewProject(const std::string& path, const std::string& name)
	{
		std::filesystem::path projectPath(path);
		projectPath.append(name);
		std::filesystem::path projectFilePath(projectPath);
		projectFilePath.append(name + ".gproj");

		if (!std::filesystem::exists(projectPath))
			std::filesystem::create_directories(projectPath);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "ProjectName";
		out << YAML::Value << name;
		out << YAML::EndMap;

		std::ofstream outStream(projectFilePath);
		outStream << out.c_str();
		outStream.close();

		OpenProject(projectFilePath.string());
		return projectFilePath.string();
	}

	ProjectSpace::ProjectSpace(const std::string& path) : m_ProjectPath(path)
	{
	}

	ProjectSpace::~ProjectSpace()
	{
	}

	void ProjectSpace::Open()
	{
		YAML::Node node = YAML::LoadFile(m_ProjectPath);
		m_ProjectName = node["ProjectName"].as<std::string>();
	}

	void ProjectSpace::Close()
	{
	}
}