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
		new PhysicsSettings(),
	};

	void ProjectSettings::Load(ProjectSpace* pProject)
	{
		for (size_t i = 0; i < 5; i++)
		{
			m_pAllSettings[i]->LoadSettings(pProject);
			ProjectSpace::SetAssetDirty(m_pAllSettings[i]->m_SettingsFile, false);
		}

		Debug::LogInfo("Loaded project settings");
	}

	void ProjectSettings::Save(ProjectSpace* pProject)
	{
		for (size_t i = 0; i < 5; i++)
		{
			m_pAllSettings[i]->SaveSettings(pProject);
			ProjectSpace::SetAssetDirty(m_pAllSettings[i]->m_SettingsFile, false);
		}

		Debug::LogInfo("Saved project settings");
	}

	void ProjectSettings::Paint(ProjectSettingsType type)
	{
		if (m_pAllSettings[size_t(type)]->OnGui())
		{
			ProjectSpace::SetAssetDirty(m_pAllSettings[size_t(type)]->m_SettingsFile);
		}
	}

	void ProjectSettings::OnStartPlay()
	{
		for (size_t i = 0; i < 5; i++)
		{
			m_pAllSettings[i]->OnStartPlay_Impl();
		}
	}

	void ProjectSettings::OnStopPlay()
	{
		for (size_t i = 0; i < 5; i++)
		{
			m_pAllSettings[i]->OnStopPlay_Impl();
		}
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
		if (!std::filesystem::exists(path))
		{
			/* TODO: Create default file? */
			return;
		}

		m_YAMLFile = YAMLFileRef{ path };
		NodeRef rootNode = m_YAMLFile.RootNodeRef();
		NodeValueRef rootValue = rootNode.ValueRef();
		if (!rootValue.Exists() || !rootValue.IsMap())
		{
			rootValue.Set(YAML::Node(YAML::NodeType::Map));
			rootValue["EditorVersion"].Set(EditorApplication::Version.GetVersionString());
		}

		OnSettingsLoaded();

		/* After migrations are done we can safely override the version */
		rootValue["EditorVersion"].Set(EditorApplication::Version.GetVersionString());
	}

	void ProjectSettings::SaveSettings(ProjectSpace* pProject)
	{
		m_YAMLFile.Save();
		OnSave(pProject);
	}

	NodeValueRef ProjectSettings::RootValue()
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

	PhysicsSettings::PhysicsSettings() : ProjectSettings("Physics.yaml")
	{
	}
}
