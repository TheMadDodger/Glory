#include "pch.h"
#include "ProjectManager.h"
#include "EditorManager.h"
#include "TemplateManager.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

namespace Glory::EditorLauncher
{
	std::vector<Project> ProjectManager::m_Projects;

    void ProjectManager::OpenProject(size_t index)
    {
        Project* pProject = &m_Projects[index];
        const std::filesystem::path editorPath = EditorManager::GetEditorLocation(pProject->SelectedVersion);
        if (!std::filesystem::exists(editorPath)) return;

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        pProject->LastEdit = timestamp;
        pProject->Version = pProject->SelectedVersion;

        std::string editorPathString = std::filesystem::absolute(editorPath).string();

        std::stringstream stream;
        stream << "cd " << editorPathString << " && start Glorious.exe " << "-projectPath=" << "\"" << pProject->Path << "\"";

        system(stream.str().c_str());

        Save();

        exit(0);
    }

    void ProjectManager::CreateProject(const ProjectCreateSettings& createSettings)
    {
        Project project;
        project.Name = createSettings.Name;
        project.Path = createSettings.Path;
        project.Version = createSettings.EditorVersion;
        project.SelectedVersion = createSettings.EditorVersion;
        m_Projects.push_back(project);

        TemplateManager::InstantiateTemplate(std::filesystem::path(project.Path).parent_path(), project.Name, createSettings.TemplateIndex);

        WriteEngineConfig(createSettings);
        WriteProjectFile(createSettings);

        OpenProject(m_Projects.size() - 1);
    }

    void ProjectManager::RemoveProjectAt(size_t index)
    {
        m_Projects.erase(m_Projects.begin() + index);
    }

    void ProjectManager::AddProject(const std::string& path)
    {
        bool open = true;

        Project project;
        YAML::Node projectNode = YAML::LoadFile(path);
        project.Name = projectNode["ProjectName"].as<std::string>();
        project.Path = path;

        std::filesystem::path projectVersionPath = project.Path;
        projectVersionPath = projectVersionPath.parent_path();
        projectVersionPath.append("ProjectSettings").append("ProjectVersion.txt");
        std::ifstream ifstream(projectVersionPath);
        if (ifstream.is_open())
        {
            std::string versionString;
            std::getline(ifstream, versionString);
            project.Version = Version::Parse(versionString.data());
            project.SelectedVersion = project.Version;
        }
        else open = false;

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        project.LastEdit = timestamp;

        m_Projects.push_back(project);
        if (open) OpenProject(m_Projects.size() - 1);
        Save();
    }

    size_t ProjectManager::ProjectCount()
    {
        return m_Projects.size();
    }

    const Project* ProjectManager::GetProject(size_t index)
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

            std::filesystem::path projectVersionPath = project.Path;
            projectVersionPath = projectVersionPath.parent_path();
            projectVersionPath.append("ProjectSettings").append("ProjectVersion.txt");
            std::ifstream ifstream(projectVersionPath);
            if (ifstream.is_open())
            {
                std::string versionString;
                std::getline(ifstream, versionString);
                project.Version = Version::Parse(versionString.data());
                project.SelectedVersion = project.Version;
            }

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

        //Load();
	}

    void ProjectManager::WriteModule(const ModuleType& moduleType, int index, YAML::Emitter& emitter)
    {
        std::vector<ModuleMetaData*> modules;
        ModuleManager::GetModulesOfType(moduleType, modules);
        std::filesystem::path path = modules[index]->Path();
        path = path.parent_path();
        std::filesystem::path::iterator end = path.end();
        --end;
        std::string name = (*end).string();
        emitter << name;
    }

    void ProjectManager::WriteEngineConfig(const ProjectCreateSettings& createSettings)
    {
        YAML::Emitter emitter;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Modules";
        emitter << YAML::Value << YAML::BeginSeq;
        WriteModule(ModuleType::MT_Window, createSettings.EngineSettings.WindowModule, emitter);
        WriteModule(ModuleType::MT_Graphics, createSettings.EngineSettings.GraphicsModule, emitter);
        WriteModule(ModuleType::MT_Renderer, createSettings.EngineSettings.RenderModule, emitter);
        WriteModule(ModuleType::MT_Input, createSettings.EngineSettings.InputModule, emitter);

        for (size_t i = 0; i < createSettings.EngineSettings.OptionalModules.size(); ++i)
        {
            int index = createSettings.EngineSettings.OptionalModules[i];
            WriteModule(ModuleType::MT_Other, index, emitter);
        }

        emitter << YAML::EndSeq;
        emitter << YAML::Key << "EditorVersion";
        std::string versionString;
        createSettings.EditorVersion.GetVersionString(versionString);
        emitter << YAML::Value << versionString;
        emitter << YAML::EndMap;

        std::filesystem::path engineConfPath = createSettings.Path;
        engineConfPath = engineConfPath.parent_path();
        engineConfPath.append("ProjectSettings");
        std::filesystem::create_directory(engineConfPath);
        engineConfPath.append("Engine.yaml");
        std::ofstream out(engineConfPath);
        out << emitter.c_str();
        out.close();
    }

    void ProjectManager::WriteProjectFile(const ProjectCreateSettings& createSettings)
    {
        if (!std::filesystem::exists(createSettings.Path)) return;
        std::ifstream fstream{ createSettings.Path, std::ios::ate };
        std::vector<char> buffer;
        const size_t fileSize = (size_t)fstream.tellg();
        buffer.resize(fileSize);
        fstream.seekg(0);
        fstream.read(buffer.data(), fileSize);
        fstream.close();
        rapidjson::Document projectFile;
        projectFile.Parse(buffer.data(), buffer.size());

        projectFile["ProjectName"].SetString(createSettings.Name.c_str(), createSettings.Name.length());

        std::ofstream out{ createSettings.Path };
        rapidjson::StringBuffer strbuf;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
        projectFile.Accept(writer);
        out << strbuf.GetString();
        out.close();

        std::filesystem::path projectFilePath = createSettings.Path;
        std::filesystem::path projectVersionPath = createSettings.Path;
        projectVersionPath = projectFilePath.parent_path();
        projectVersionPath.append("ProjectSettings").append("ProjectVersion.txt");
        std::ofstream fileStream(projectVersionPath, std::ofstream::out | std::ofstream::trunc);
        std::string versionString;
        createSettings.EditorVersion.GetVersionString(versionString);
        fileStream.write(versionString.c_str(), versionString.size());
        fileStream.close();
    }

	ProjectManager::ProjectManager()
	{
	}

	ProjectManager::~ProjectManager()
	{
	}
}
