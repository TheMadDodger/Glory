#pragma once
#include "ProjectSpace.h"
#include "Undo.h"

#include <string_view>

#include <yaml-cpp/yaml.h>
#include <NodeRef.h>

#define SETTINGS_DEFAULT_KEY(node, key, nodeType) Utils::NodeValueRef node = RootValue()[key]; \
if (!node.Exists() || !node.Is##nodeType()) \
{ \
	node.Set(YAML::Node(YAML::NodeType::nodeType)); \
}

namespace Glory::Editor
{
	struct ActionRecord;

    class ProjectSettings
    {
	public:
		GLORY_EDITOR_API static void Load(ProjectSpace* pProject);
		GLORY_EDITOR_API static void Save(ProjectSpace* pProject);

		GLORY_EDITOR_API static void Paint(size_t index);
		GLORY_EDITOR_API static void Open(size_t index);

		GLORY_EDITOR_API static void OnStartPlay();
		GLORY_EDITOR_API static void OnStopPlay();

		GLORY_EDITOR_API static void Add(ProjectSettings* pSettings);
		GLORY_EDITOR_API static size_t SettingsCount();
		GLORY_EDITOR_API static std::string_view Name(size_t index);
		GLORY_EDITOR_API static ProjectSettings* Get(const std::string_view name);

		GLORY_EDITOR_API Utils::YAMLFileRef& operator*();

		GLORY_EDITOR_API static void Compile(const std::filesystem::path& path);
		GLORY_EDITOR_API static void CreateCompileTask();

    protected:
        GLORY_EDITOR_API ProjectSettings(const char* settingsFile);
        GLORY_EDITOR_API virtual ~ProjectSettings();

	protected:
        virtual bool OnGui() = 0;
		virtual void OnOpen() {};
		virtual void OnSettingsLoaded() {}
		virtual void OnSave(ProjectSpace* pProject) {}
		virtual void OnCompile(const std::filesystem::path& path) {}
		void LoadSettings(ProjectSpace* pProject);
		void SaveSettings(ProjectSpace* pProject);

		virtual void OnStartPlay_Impl() {}
		virtual void OnStopPlay_Impl() {}

		GLORY_EDITOR_API Utils::NodeValueRef RootValue();

    protected:
		Utils::YAMLFileRef m_YAMLFile;
		const char* m_Name;
    };

	class GeneralSettings : public ProjectSettings
	{
	public:
		GeneralSettings();

	private:
		virtual bool OnGui() override;
		virtual void OnSettingsLoaded() override;
		virtual void OnCompile(const std::filesystem::path& path) override;
		virtual void OnStartPlay_Impl() override;
	};

	class EngineSettings : public ProjectSettings
	{
	public:
		EngineSettings();

	private:
		virtual bool OnGui() override;
		void OnSave(ProjectSpace* pProject) override;

		void DrawLeftPanel();
		bool DrawRightPanel();

	private:
		size_t m_MenuIndex;
	};

	class LayerSettings : public ProjectSettings
	{
	public:
		LayerSettings();

	private:
		virtual bool OnGui() override;
		virtual void OnSettingsLoaded() override;
		virtual void OnCompile(const std::filesystem::path& path) override;
	};

	class InputSettings : public ProjectSettings
	{
	public:
		InputSettings();

	private:
		virtual bool OnGui() override;
		virtual void OnSettingsLoaded() override;
		virtual void OnCompile(const std::filesystem::path& path) override;

		virtual void OnStartPlay_Impl() override;
		virtual void OnStopPlay_Impl() override;
	};

	class PackageSettings : public ProjectSettings
	{
	public:
		PackageSettings();
		void VerifySettings();

	private:
		virtual bool OnGui() override;
		virtual void OnSettingsLoaded() override;
	};

	class RenderSettings : public ProjectSettings
	{
	public:
		RenderSettings();
		void VerifySettings();

	private:
		virtual bool OnGui() override;
		virtual void OnOpen() override;
		virtual void OnSettingsLoaded() override;
		virtual void OnCompile(const std::filesystem::path& path) override;
		virtual void OnStartPlay_Impl() override;

		void SendToRenderer();

		void AddNewPipelines();

		class PipelineReorderAction : public IAction
		{
		public:
			PipelineReorderAction(RenderSettings* pRenderSettings, UUID movedPipeline, size_t oldIndex, size_t newIndex);
			virtual ~PipelineReorderAction() = default;

		private:
			void OnUndo(const ActionRecord& actionRecord) override;
			void OnRedo(const ActionRecord& actionRecord) override;

		private:
			RenderSettings* m_pRenderSettings;
			const UUID m_MovedPipeline;
			const size_t m_OldIndex;
			const size_t m_NewIndex;
		};
	};
}