#include "ProjectSettings.h"
#include "EditorApplication.h"

#include <Debug.h>

#include <filesystem>
#include <fstream>

namespace Glory::Editor
{
	GeneralSettings General;
	EngineSettings Engine;
	LayerSettings Layer;
	InputSettings Input;

	std::vector<ProjectSettings*> Settings = {
		&General,
		&Engine,
		&Layer,
		&Input,
	};

	void ProjectSettings::Load(ProjectSpace* pProject)
	{
		for (size_t i = 0; i < Settings.size(); ++i)
		{
			Settings[i]->LoadSettings(pProject);
			ProjectSpace::SetAssetDirty(Settings[i]->m_SettingsFile, false);
		}

		EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo("Loaded project settings");
	}

	void ProjectSettings::Save(ProjectSpace* pProject)
	{
		for (size_t i = 0; i < Settings.size(); ++i)
		{
			Settings[i]->SaveSettings(pProject);
			ProjectSpace::SetAssetDirty(Settings[i]->m_SettingsFile, false);
		}

		EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo("Saved project settings");
	}

	void ProjectSettings::Paint(ProjectSettingsType type)
	{
		if (Settings[size_t(type)]->OnGui())
		{
			ProjectSpace::SetAssetDirty(Settings[size_t(type)]->m_SettingsFile);
		}
	}

	void ProjectSettings::OnStartPlay()
	{
		for (size_t i = 0; i < Settings.size(); ++i)
		{
			Settings[i]->OnStartPlay_Impl();
		}
	}

	void ProjectSettings::OnStopPlay()
	{
		for (size_t i = 0; i < Settings.size(); ++i)
		{
			Settings[i]->OnStopPlay_Impl();
		}
	}

	void ProjectSettings::Add(ProjectSettings* pSettings)
	{
		Settings.push_back(pSettings);
	}

	ProjectSettings::ProjectSettings(const char* settingsFile)
		: m_YAMLFile(), m_SettingsFile(settingsFile)
	{
	}

	ProjectSettings::~ProjectSettings()
	{
	}

	void ProjectSettings::LoadSettings(ProjectSpace* pProject)
	{
		std::filesystem::path path = pProject->SettingsPath();
		path.append(m_SettingsFile);
		m_YAMLFile = Utils::YAMLFileRef{ path };
		
		if (!std::filesystem::exists(path))
		{
			/* TODO: Create default file? */
			return;
		}

		Utils::NodeRef rootNode = m_YAMLFile.RootNodeRef();
		Utils::NodeValueRef rootValue = rootNode.ValueRef();
		if (!rootValue.Exists() || !rootValue.IsMap())
		{
			rootValue.Set(YAML::Node(YAML::NodeType::Map));

			std::string versionString;
			EditorApplication::Version.GetVersionString(versionString);
			rootValue["EditorVersion"].Set(versionString);
		}

		OnSettingsLoaded();

		/* After migrations are done we can safely override the version */
		std::string versionString;
		EditorApplication::Version.GetVersionString(versionString);
		rootValue["EditorVersion"].Set(versionString);
	}

	void ProjectSettings::SaveSettings(ProjectSpace* pProject)
	{
		m_YAMLFile.Save();
		OnSave(pProject);
	}

	Utils::NodeValueRef ProjectSettings::RootValue()
	{
		return m_YAMLFile.RootNodeRef().ValueRef();
	}

	GeneralSettings::GeneralSettings() : ProjectSettings("General.yaml")
	{
	}

	bool GeneralSettings::OnGui()
	{
		ImGui::TextUnformatted("Comming soon!");
		return false;
	}

	EngineSettings::EngineSettings() : ProjectSettings("Engine.yaml"), m_MenuIndex(0)
	{
	}

	LayerSettings::LayerSettings() : ProjectSettings("Layers.yaml")
	{
	}

	InputSettings::InputSettings() : ProjectSettings("Input.yaml")
	{
	}
}
