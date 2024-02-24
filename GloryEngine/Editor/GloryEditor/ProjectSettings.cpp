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
	PackageSettings Packaging;

	std::vector<ProjectSettings*> Settings = {
		&General,
		&Engine,
		&Layer,
		&Input,
		&Packaging,
	};

	void ProjectSettings::Load(ProjectSpace* pProject)
	{
		for (size_t i = 0; i < Settings.size(); ++i)
		{
			Settings[i]->LoadSettings(pProject);
			ProjectSpace::SetAssetDirty(Settings[i]->m_Name, false);
		}

		EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo("Loaded project settings");
	}

	void ProjectSettings::Save(ProjectSpace* pProject)
	{
		for (size_t i = 0; i < Settings.size(); ++i)
		{
			Settings[i]->SaveSettings(pProject);
			ProjectSpace::SetAssetDirty(Settings[i]->m_Name, false);
		}

		EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo("Saved project settings");
	}

	void ProjectSettings::Paint(size_t index)
	{
		if (Settings[index]->OnGui())
		{
			ProjectSpace::SetAssetDirty(Settings[index]->m_Name);
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

	size_t ProjectSettings::SettingsCount()
	{
		return Settings.size();
	}

	std::string_view ProjectSettings::Name(size_t index)
	{
		return { Settings[index]->m_Name };
	}

	ProjectSettings* ProjectSettings::Get(const std::string_view name)
	{
		for (size_t i = 0; i < Settings.size(); ++i)
		{
			const std::string_view other{ Settings[i]->m_Name };
			if (name == other) return Settings[i];
		}
		return nullptr;
	}

	Utils::YAMLFileRef& ProjectSettings::operator*()
	{
		return m_YAMLFile;
	}

	ProjectSettings::ProjectSettings(const char* settingsFile)
		: m_YAMLFile(), m_Name(settingsFile)
	{
	}

	ProjectSettings::~ProjectSettings()
	{
	}

	void ProjectSettings::LoadSettings(ProjectSpace* pProject)
	{
		std::filesystem::path path = pProject->SettingsPath();
		path.append(m_Name).replace_extension("yaml");
		m_YAMLFile = Utils::YAMLFileRef{ path };

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

	GeneralSettings::GeneralSettings() : ProjectSettings("General")
	{
	}

	bool GeneralSettings::OnGui()
	{
		ImGui::TextUnformatted("Comming soon!");
		return false;
	}

	EngineSettings::EngineSettings() : ProjectSettings("Engine"), m_MenuIndex(0)
	{
	}

	LayerSettings::LayerSettings() : ProjectSettings("Layers")
	{
	}

	InputSettings::InputSettings() : ProjectSettings("Input")
	{
	}

	PackageSettings::PackageSettings() : ProjectSettings("Packaging")
	{
	}
}
