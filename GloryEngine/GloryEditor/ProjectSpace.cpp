#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <AssetDatabase.h>
#include "ContentBrowser.h"
#include "ProjectSpace.h"
#include "EditorApplication.h"

namespace Glory::Editor
{
	ProjectSpace* ProjectSpace::m_pCurrentProject = nullptr;
	std::mutex ProjectSpace::m_ProjectLock;

	GLORY_EDITOR_API void ProjectSpace::OpenProject(const std::string& path)
	{
		CloseProject();
		std::unique_lock<std::mutex> lock(m_ProjectLock);
		m_pCurrentProject = new ProjectSpace(path);
		lock.unlock();
		m_pCurrentProject->Open();
		EditorAssetLoader::Start();
	}

	GLORY_EDITOR_API void ProjectSpace::CloseProject()
	{
		if (m_pCurrentProject == nullptr) return;
		EditorAssetLoader::Stop();
		m_pCurrentProject->Close();
		std::unique_lock<std::mutex> lock(m_ProjectLock);
		delete m_pCurrentProject;
		m_pCurrentProject = nullptr;
		lock.unlock();
	}

	GLORY_EDITOR_API ProjectSpace* ProjectSpace::GetOpenProject()
	{
		std::unique_lock<std::mutex> lock(m_ProjectLock);
		ProjectSpace* pProject = m_pCurrentProject;
		lock.unlock();
		return pProject;
	}

	GLORY_EDITOR_API bool ProjectSpace::ProjectExists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	GLORY_EDITOR_API bool ProjectSpace::ProjectExists(const std::string& path, const std::string& name)
	{
		std::filesystem::path projectPath(path);
		projectPath.append(name);
		projectPath.append(name + ".gproj");
		return ProjectExists(projectPath.string());
	}

	GLORY_EDITOR_API std::string ProjectSpace::NewProject(const std::string& path, const std::string& name)
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

	GLORY_EDITOR_API std::string ProjectSpace::Name()
	{
		return m_ProjectName;
	}

	GLORY_EDITOR_API std::string ProjectSpace::RootPath()
	{
		return m_ProjectRootPath;
	}

	GLORY_EDITOR_API std::string ProjectSpace::ProjectPath()
	{
		return m_ProjectFilePath;
	}

	GLORY_EDITOR_API std::string ProjectSpace::CachePath()
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
		std::unique_lock<std::mutex> lock(m_ProjectLock);
		CreateFolder("Assets");
		ImportModuleAssets(false);
		CreateFolder("Cache");
		CreateFolder("Cache/ShaderSource");
		CreateFolder("Cache/CompiledShaders");
		YAML::Node node = YAML::LoadFile(m_ProjectFilePath);
		m_ProjectName = node["ProjectName"].as<std::string>();
		lock.unlock();

		AssetDatabase::Load();
		ContentBrowser::LoadProject();
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

	void ProjectSpace::ImportModuleAssets(bool overwrite)
	{
		std::filesystem::path path = "./ModuleAssets/";
		std::filesystem::path moduleAssetsPath = RootPath() + "\\ModuleAssets\\";
		if (!overwrite && std::filesystem::exists(moduleAssetsPath)) return;
		std::filesystem::copy(path, moduleAssetsPath, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
	}
}