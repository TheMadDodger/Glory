#pragma once
#include "ProjectSpace.h"
#include <string_view>
#include <yaml-cpp/yaml.h>

#define SETTINGS_DEFAULT_KEY(node, key, nodeType) YAML::Node node = m_SettingsNode[key]; \
if (!node.IsDefined() || !node.Is##nodeType()) \
{ \
	node = YAML::Node(YAML::NodeType::nodeType); \
}

namespace Glory::Editor
{
	enum class ProjectSettingsType
	{
		General,
		Engine,
		Layers,
		Input,
		Count
	};

    class ProjectSettings
    {
	public:
		static void Load(ProjectSpace* pProject);
		static void Save(ProjectSpace* pProject);

		static void Paint(ProjectSettingsType type);

		static void OnStartPlay();
		static void OnStopPlay();

    protected:
        ProjectSettings(const char* settingsFile);
        virtual ~ProjectSettings();

	protected:
        virtual void OnGui() = 0;
		virtual void OnSettingsLoaded() {}
		void LoadSettings(ProjectSpace* pProject);
		void SaveSettings(ProjectSpace* pProject);

		virtual void OnStartPlay_Impl() {}
		virtual void OnStopPlay_Impl() {}

    protected:
        YAML::Node m_SettingsNode;
		const char* m_SettingsFile;

		static ProjectSettings* m_pAllSettings[];
    };

	class GeneralSettings : public ProjectSettings
	{
	public:
		GeneralSettings();
		//~GeneralSettings();

	private:
		virtual void OnGui() override {}
	};

	class EngineSettings : public ProjectSettings
	{
	public:
		EngineSettings();

	private:
		virtual void OnGui() override {}
	};

	class LayerSettings : public ProjectSettings
	{
	public:
		LayerSettings();

	private:
		virtual void OnGui() override {}
	};

	class InputSettings : public ProjectSettings
	{
	public:
		InputSettings();

	private:
		virtual void OnGui() override;
		virtual void OnSettingsLoaded() override;

		virtual void OnStartPlay_Impl() override;
		virtual void OnStopPlay_Impl() override;
	};
}