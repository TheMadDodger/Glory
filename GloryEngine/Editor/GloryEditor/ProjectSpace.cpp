#include "ProjectSpace.h"

#include "FileBrowser.h"
#include "EditorApplication.h"
#include "ProjectSettings.h"
#include "EditorAssetDatabase.h"
#include "TitleBar.h"

#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <AssetDatabase.h>

namespace Glory::Editor
{
	ProjectSpace* ProjectSpace::m_pCurrentProject = nullptr;
	std::mutex ProjectSpace::m_ProjectLock;
	std::unordered_map<ProjectCallback, std::vector<std::function<void(ProjectSpace*)>>> ProjectSpace::m_ProjectCallbacks;

	void ProjectSpace::OpenProject(const std::string& path)
	{
		std::string absolutePath = std::filesystem::absolute(path).string();
		CloseProject();
		std::unique_lock<std::mutex> lock(m_ProjectLock);
		m_pCurrentProject = new ProjectSpace(absolutePath);
		lock.unlock();
		m_pCurrentProject->Open();
		EditorAssetLoader::Start();
	}

	void ProjectSpace::CloseProject()
	{
		if (m_pCurrentProject == nullptr) return;
		EditorAssetLoader::Stop();
		m_pCurrentProject->Close();
		std::unique_lock<std::mutex> lock(m_ProjectLock);
		delete m_pCurrentProject;
		m_pCurrentProject = nullptr;
		lock.unlock();
	}

	ProjectSpace* ProjectSpace::GetOpenProject()
	{
		std::unique_lock<std::mutex> lock(m_ProjectLock);
		ProjectSpace* pProject = m_pCurrentProject;
		lock.unlock();
		return pProject;
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

	const std::string& ProjectSpace::Name() const
	{
		return m_ProjectName;
	}

	const std::string& ProjectSpace::RootPath() const
	{
		return m_ProjectRootPath;
	}

	const std::string& ProjectSpace::ModuleSettingsPath() const
	{
		return m_ModuleSettingsPath;
	}

	const std::string& ProjectSpace::ProjectPath() const
	{
		return m_ProjectFilePath;
	}

	const std::string& ProjectSpace::CachePath() const
	{
		if (!std::filesystem::exists(m_CachePath)) std::filesystem::create_directory(m_CachePath);
		return m_CachePath;
	}

	const std::string& ProjectSpace::LibraryPath() const
	{
		if (!std::filesystem::exists(m_LibraryPath)) std::filesystem::create_directory(m_LibraryPath);
		return m_LibraryPath;
	}

	const std::string& ProjectSpace::SettingsPath() const
	{
		if (!std::filesystem::exists(m_SettingsPath)) std::filesystem::create_directory(m_SettingsPath);
		return m_SettingsPath;
	}

	void ProjectSpace::RegisterCallback(const ProjectCallback& callbackType, std::function<void(ProjectSpace*)> callback)
	{
		m_ProjectCallbacks[callbackType].push_back(callback);
	}

	void ProjectSpace::SetAssetDirty(const char* key, bool dirty)
	{
		ProjectSpace* pOpenProject = GetOpenProject();
		if (!pOpenProject) return;
		auto itor = std::find(pOpenProject->m_DirtyKeys.begin(), pOpenProject->m_DirtyKeys.end(), key);
		if (dirty)
		{
			if (itor != pOpenProject->m_DirtyKeys.end()) return; // Already dirty
			pOpenProject->m_DirtyKeys.emplace_back(key);
			TitleBar::SetText("ProjectChanges", "*");
			return;
		}
		if (itor == pOpenProject->m_DirtyKeys.end()) return;
		pOpenProject->m_DirtyKeys.erase(itor);
		TitleBar::SetText("ProjectChanges", pOpenProject->m_DirtyKeys.size() > 0 ? "*" : "");
	}

	bool ProjectSpace::HasUnsavedChanges()
	{
		ProjectSpace* pProject = GetOpenProject();
		if (!pProject) return false;
		return pProject->m_DirtyKeys.size() > 0;
	}

	ProjectSpace::ProjectSpace(const std::string& path)
		: m_ProjectFilePath(path), m_ProjectRootPath(std::filesystem::path(path).parent_path().string()),
		m_CachePath(std::filesystem::path(path).parent_path().append("Cache").string()), m_LibraryPath(std::filesystem::path(path).parent_path().append("Library").string()),
		m_SettingsPath(std::filesystem::path(path).parent_path().append("ProjectSettings").string()),
		m_ModuleSettingsPath(std::filesystem::path(path).parent_path().append("Modules").string())
	{
	}

	ProjectSpace::~ProjectSpace()
	{
	}

	void ProjectSpace::Open()
	{
		std::unique_lock<std::mutex> lock(m_ProjectLock);
		CreateFolder("Assets");
		CreateFolder("Cache");
		CreateFolder("Cache/ShaderSource");
		CreateFolder("Cache/CompiledShaders");
		CreateFolder("Modules");
		m_ProjectFileNode = YAML::LoadFile(m_ProjectFilePath);
		m_ProjectName = m_ProjectFileNode["ProjectName"].as<std::string>();

		for (size_t i = 0; i < m_ProjectCallbacks[ProjectCallback::OnOpen].size(); i++)
		{
			m_ProjectCallbacks[ProjectCallback::OnOpen][i](this);
		}

		lock.unlock();

		EditorAssetDatabase::Load(m_ProjectFileNode);
		EditorAssetDatabase::ImportModuleAssets();
		FileBrowser::LoadProject();
		ProjectSettings::Load(this);

		TitleBar::SetText("Project", m_ProjectName.c_str());

		std::stringstream stream;
		stream << "Opened project at: " << m_ProjectFilePath;
		Debug::LogInfo(stream.str());
	}

	void ProjectSpace::Close()
	{
		for (size_t i = 0; i < m_ProjectCallbacks[ProjectCallback::OnClose].size(); i++)
		{
			m_ProjectCallbacks[ProjectCallback::OnClose][i](this);
		}

		TitleBar::SetText("Project", "No Project open");
		m_DirtyKeys.clear();
		TitleBar::SetText("ProjectChanges", "");

		AssetDatabase::Clear();

		Debug::LogInfo("Closed current project");
	}

	void ProjectSpace::CreateFolder(const std::string& name)
	{
		std::filesystem::path path = m_ProjectRootPath;
		path.append(name);
		if (std::filesystem::exists(path)) return;
		std::filesystem::create_directories(path);
	}

	void ProjectSpace::Save()
	{
		ProjectSpace* pProject = GetOpenProject();
		if (pProject == nullptr) return;
		pProject->SaveProject();
	}

	void ProjectSpace::SaveProject()
	{
		/* Save project file */
		YAML::Emitter projectOut;
		projectOut << m_ProjectFileNode;
		std::ofstream projectFileStream(m_ProjectFilePath);
		projectFileStream << projectOut.c_str();
		projectFileStream.close();

		/* Save project version */
		std::filesystem::path projectVersionTxtPath = m_SettingsPath;
		projectVersionTxtPath.append("ProjectVersion.txt");
		std::ofstream versionFileStream(projectVersionTxtPath, std::ofstream::out | std::ofstream::trunc);
		std::string versionString = Glory::Editor::Version.GetVersionString();
		versionFileStream.write(versionString.c_str(), versionString.size());
		versionFileStream.close();

		/* Save project settings */
		ProjectSettings::Save(this);

		m_DirtyKeys.clear();
		TitleBar::SetText("ProjectChanges", "");

		EditorAssetDatabase::SaveDirtyAssets();
		EditorAssetDatabase::Reload();

		std::stringstream stream;
		stream << "Project saved to: " << m_ProjectFilePath;
		Debug::LogInfo(stream.str());
	}
}