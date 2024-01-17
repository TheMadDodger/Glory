#pragma once
#include "ModuleManager.h"
#include "ProjectManager.h"

namespace Glory::EditorLauncher
{
	enum NewProjectMenu : int
	{
		General,
		Engine,

		NP_MAX,
	};

	enum class ProjectValidationResult
	{
		EVR_Success,
		EVR_InvalidName,
		EVR_AlreadyExists,
		EVR_NoEditor,
		EVR_MissingModules,
		EVR_DuplicateModules,
		EVR_NoTemplate,
	};

	class NewProjectWindow
	{
	public:
		NewProjectWindow();
		virtual ~NewProjectWindow();

		bool Draw();
		void Open();
		void Close();

	private:
		void DrawHeader();
		void DrawLeftPanel();
		void DrawRightPanel();
		void GeneralSettingsMenu();
		void EngineSettingsMenu();

		ProjectValidationResult ValidateSettings();
		bool ValidateModule(const ModuleType& moduleType, const int& currentIndex);

		bool DrawModuleSelector(const char* name, const ModuleType& moduleType, int& currentIndex, float posX, float maxSize, bool showErrorOnInvalid = true);
		void DrawOptionalModulesArray(ImVec2 regionAvail, float maxItemSize);

		bool ProjectExists(const std::string& path, const std::string& name);
		std::filesystem::path GetProjectPath(const std::string& path, const std::string& name);

	private:
		bool m_IsOpen;
		int m_SelectedEditorIndex;

		char m_PathText[100];
		char m_ProjectNameText[100] = "New Project";

		std::string m_BrowsingPath;
		std::string m_ProjectFolder;
		std::string m_DefaultProjectsFolder;

		const char* FILEDIALOG_ID = "FileDialog";

		NewProjectMenu m_CurrentMenu = NewProjectMenu::General;

		EngineSettings m_EngineSettings;

		int m_SelectedTemplate;

		bool m_Valid;

		static const size_t MINPROJECTNAMELENGTH = 3;
	};
}
