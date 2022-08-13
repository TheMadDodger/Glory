#include "pch.h"
#include "ProjectManager.h"

namespace Glory::EditorLauncher
{
	std::vector<Project> ProjectManager::m_Projects;

    void ProjectManager::OpenProject(size_t index)
    {
        Project* pProject = &m_Projects[index];

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        pProject->LastEdit = timestamp;

        Save();
    }

    void ProjectManager::CreateProject(const ProjectCreateSettings& createSettings)
    {
        Project project;
        project.Name = createSettings.Name;
        project.Path = createSettings.Path;
        m_Projects.push_back(project);

        // Do stuff to set up the project

        OpenProject(m_Projects.size() - 1);
        Save();
    }

    void ProjectManager::RemoveProjectAt(size_t index)
    {
        m_Projects.erase(m_Projects.begin() + index);
    }

    void ProjectManager::AddProject(const std::string& path)
    {
        Project project;
        YAML::Node projectNode = YAML::LoadFile(path);
        project.Name = projectNode["ProjectName"].as<std::string>();
        project.Path = path;
        m_Projects.push_back(project);
        OpenProject(m_Projects.size() - 1);
        Save();
    }

    size_t ProjectManager::ProjectCount()
    {
        return m_Projects.size();
    }

    const Project* const ProjectManager::GetProject(size_t index)
    {
        return &m_Projects[index];
    }

    void ProjectManager::Load()
	{
        m_Projects.clear();
        std::filesystem::path path("Projects.list");
        if (!std::filesystem::exists(path)) return;
        YAML::Node node = YAML::LoadFile(path.string());
        for (size_t i = 0; i < node.size(); i++)
        {
            YAML::Node element = node[i];
            Project project;
            project.Name = element["ProjectName"].as<std::string>();
            project.Path = element["ProjectPath"].as<std::string>();
            if (element["LastEdit"].IsDefined()) project.LastEdit = element["LastEdit"].as<long long>();
            else
            {
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
                project.LastEdit = timestamp;
            }
            m_Projects.push_back(project);
        }
	}

	void ProjectManager::Save()
	{
        std::filesystem::path path("Projects.list");
        YAML::Emitter out;

        out << YAML::BeginSeq;
        for (size_t i = 0; i < m_Projects.size(); i++)
        {
            Project* pProject = &m_Projects[i];
            out << YAML::BeginMap;
            out << YAML::Key << "ProjectName";
            out << YAML::Value << pProject->Name;
            out << YAML::Key << "ProjectPath";
            out << YAML::Value << pProject->Path;
            out << YAML::Key << "LastEdit";
            out << YAML::Value << pProject->LastEdit;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        std::ofstream outStream(path);
        outStream << out.c_str();
        outStream.close();

        Load();
	}

	ProjectManager::ProjectManager()
	{
	}

	ProjectManager::~ProjectManager()
	{
	}
}
