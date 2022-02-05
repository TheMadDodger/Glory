#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <AssetDatabase.h>
#include "ProjectSpace.h"
#include "EditorApplication.h"

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

	std::string ProjectSpace::Name()
	{
		return m_ProjectName;
	}

	std::string ProjectSpace::RootPath()
	{
		return m_ProjectRootPath;
	}

	std::string ProjectSpace::ProjectPath()
	{
		return m_ProjectFilePath;
	}

	std::string ProjectSpace::CachePath()
	{
		return m_CachePath;
	}

	ProjectSpace::ProjectSpace(const std::string& path)
		: m_ProjectFilePath(path), m_ProjectRootPath(std::filesystem::path(path).parent_path().string()), m_CachePath(std::filesystem::path(path).parent_path().string() + "\\Cache\\")
	{
	}

	ProjectSpace::~ProjectSpace()
	{
	}

	void ProjectSpace::Open()
	{
		CreateFolder("Assets");
		CreateFolder("Cache");
		YAML::Node node = YAML::LoadFile(m_ProjectFilePath);
		m_ProjectName = node["ProjectName"].as<std::string>();

		AssetDatabase::Load();
		EditorApplication::GetInstance()->GetMainEditor()->GetAssetLoader()->LoadAssets();
	}

	void ProjectSpace::Close()
	{
		AssetDatabase::Save();
	}

	void ProjectSpace::CreateFolder(const std::string& name)
	{
		std::filesystem::path path = m_ProjectRootPath;
		path.append(name);
		if (std::filesystem::exists(path)) return;
		std::filesystem::create_directories(path);
	}
}