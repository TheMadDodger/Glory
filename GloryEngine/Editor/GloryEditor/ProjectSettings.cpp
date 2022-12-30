#include "ProjectSettings.h"
#include "EditorApplication.h"
#include <filesystem>
#include <fstream>

namespace Glory::Editor
{
	ProjectSettings* ProjectSettings::m_pAllSettings[] = {
		new GeneralSettings(),
		new EngineSettings(),
		new LayerSettings(),
		new InputSettings(),
	};

	void ProjectSettings::Load(ProjectSpace* pProject)
	{
		for (size_t i = 0; i < 4; i++)
		{
			m_pAllSettings[i]->LoadSettings(pProject);
		}
	}

	void ProjectSettings::Save(ProjectSpace* pProject)
	{
		for (size_t i = 0; i < 4; i++)
		{
			m_pAllSettings[i]->SaveSettings(pProject);
		}
	}

	void ProjectSettings::Paint(ProjectSettingsType type)
	{
		m_pAllSettings[size_t(type)]->OnGui();
	}

	void ProjectSettings::OnStartPlay()
	{
		for (size_t i = 0; i < 4; i++)
		{
			m_pAllSettings[i]->OnStartPlay_Impl();
		}
	}

	void ProjectSettings::OnStopPlay()
	{
		for (size_t i = 0; i < 4; i++)
		{
			m_pAllSettings[i]->OnStopPlay_Impl();
		}
	}

	ProjectSettings::ProjectSettings(const char* settingsFile)
		: m_SettingsNode(), m_SettingsFile(settingsFile)
	{
	}

	ProjectSettings::~ProjectSettings()
	{
	}

	void ProjectSettings::LoadSettings(ProjectSpace* pProject)
	{
		std::filesystem::path path = pProject->SettingsPath();
		path.append(m_SettingsFile);
		if (!std::filesystem::exists(path))
		{
			/* TODO: Create default file? */
			return;
		}

		m_SettingsNode = YAML::LoadFile(path.string());
		if (!m_SettingsNode.IsDefined() || !m_SettingsNode.IsMap())
		{
			m_SettingsNode = YAML::Node(YAML::NodeType::Map);
			m_SettingsNode["EditorVersion"] = EditorApplication::Version.GetVersionString();
		}

		OnSettingsLoaded();

		/* After migrations are done we can safely override the version */
		m_SettingsNode["EditorVersion"] = EditorApplication::Version.GetVersionString();
	}

	void ProjectSettings::SaveSettings(ProjectSpace* pProject)
	{
		std::filesystem::path path = pProject->SettingsPath();
		path.append(m_SettingsFile);

		YAML::Emitter out;
		out << m_SettingsNode;

		std::ofstream fileStream(path);
		fileStream << out.c_str();
		fileStream.close();
	}

	GeneralSettings::GeneralSettings() : ProjectSettings("General.yaml")
	{
	}

	void GeneralSettings::OnGui()
	{
		ImGui::TextUnformatted("Comming soon!");
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
