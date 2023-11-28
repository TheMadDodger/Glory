#include "pch.h"
#include "NewProjectWindow.h"
#include "EditorManager.h"
#include "LauncherHub.h"
#include "ProjectManager.h"
#include "TemplateManager.h"

#include <tinyfiledialogs.h>
#include <imgui.h>
#include <ShlObj_core.h>

namespace Glory::EditorLauncher
{
	const std::map<NewProjectMenu, const char*> NEWPROJECTMENUTOSTRING = {
		{ NewProjectMenu::General, "General" },
		{ NewProjectMenu::Engine, "Engine" },
	};

	const std::map<std::string, const char*> TEMPLATE_ICONS = {
		{ "Empty", ICON_FA_HANDS_HOLDING_CIRCLE },
		{ "Default", ICON_FA_CUBES },
	};

	const char* UNKNOWN_TEMPLATE_ICON = ICON_FA_WAND_MAGIC_SPARKLES;

	NewProjectWindow::NewProjectWindow() : m_IsOpen(false), m_SelectedEditorIndex(-1), m_Valid(false), m_SelectedTemplate(-1)
	{
		/* FIXME: Need a platform independant solution for this */
		wchar_t* p;
		if (S_OK != SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &p))
		{
			m_DefaultProjectsFolder = "C:/";
			return;
		}

		std::filesystem::path result = p;
		CoTaskMemFree(p);

		result.append("Glorious\\Projects\\");

		m_DefaultProjectsFolder = result.string();

		if (!std::filesystem::exists(m_DefaultProjectsFolder))
		{
			std::filesystem::create_directories(m_DefaultProjectsFolder);
		}
	}

	NewProjectWindow::~NewProjectWindow()
	{
	}

	bool NewProjectWindow::Draw()
	{
		if (!m_IsOpen) return false;

		DrawLeftPanel();
		DrawRightPanel();

		return true;
	}

	void NewProjectWindow::Open()
	{
		m_BrowsingPath = m_DefaultProjectsFolder;
		strcpy(m_PathText, m_BrowsingPath.data());
		m_IsOpen = true;

		m_SelectedEditorIndex = -1;
		m_SelectedTemplate = -1;
		TemplateManager::Clear();
	}

	void NewProjectWindow::Close()
	{
		m_IsOpen = false;
	}

	void NewProjectWindow::DrawHeader()
	{
		ImGui::BeginChild("Header", ImVec2(0.0f, 50.0f), true);

		ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		float maxHeight = regionAvail.y;

		ImVec2 buttonSize = ImVec2(100.0f, maxHeight);

		ProjectValidationResult result = ValidateSettings();
		bool valid = result == ProjectValidationResult::EVR_Success;

		ImGui::BeginDisabled(!valid);
		if (ImGui::Button("Create Project", buttonSize))
		{
			m_CurrentMenu = NewProjectMenu::General;

			const EditorInfo& editorInfo = EditorManager::GetEditorInfo(m_SelectedEditorIndex);

			ProjectCreateSettings createSettings{};
			createSettings.Name = m_ProjectNameText;
			createSettings.Path = GetProjectPath(m_PathText, m_ProjectNameText).string();
			createSettings.EngineSettings = m_EngineSettings;
			createSettings.EditorVersion = editorInfo.Version;
			createSettings.TemplateIndex = m_SelectedTemplate;

			ProjectManager::CreateProject(createSettings);
			Close();
		}
		ImGui::EndDisabled();

		if (result != ProjectValidationResult::EVR_Success)
		{
			std::string errorText = "";

			ImGui::SameLine();
			switch (result)
			{
			case ProjectValidationResult::EVR_InvalidName:
				errorText = "General: Invalid project name";
				break;
			case ProjectValidationResult::EVR_NoEditor:
				errorText = "General: No editor selected";
				break;
			case ProjectValidationResult::EVR_MissingModules:
				errorText = "Engine: Missing modules";
				break;
			case ProjectValidationResult::EVR_DuplicateModules:
				errorText = "Engine: Duplicate optional modules";
				break;
			case ProjectValidationResult::EVR_DuplicateScriptingModules:
				errorText = "Engine: Duplicate scripting modules";
				break;
			case ProjectValidationResult::EVR_AlreadyExists:
				errorText = "General: Project already exists at path";
				break;
			case ProjectValidationResult::EVR_NoTemplate:
				errorText = "General: Select a template";
				break;
			default:
				errorText = "Unknown reason";
				break;
			}
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), errorText.c_str());
		}

		ImGui::SameLine(regionAvail.x - buttonSize.x + 8.0f);
		if (ImGui::Button("Cancel", buttonSize))
		{
			Close();
		}

		ImGui::EndChild();
	}

	void NewProjectWindow::DrawLeftPanel()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::PushFont(LauncherHub::BoldLargeFont);
		ImGui::BeginChild("LeftPanel", ImVec2(250.0f, 0.0f), true);
		ImGui::BeginChild("LeftPanelHeader", ImVec2(0.0f, 50.0f), false);

		ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
		float size = LauncherHub::BoldLargeFont->FontSize;
		float cursorPosY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (size / 2.0f) + (contentRegionAvail.y / 2.0f));
		ImGui::TextUnformatted("New Project");

		ImGui::EndChild();

		ImGui::BeginChild("LeftPanelBody", ImVec2(0.0f, 0.0f), false);

		for (size_t i = 0; i < NewProjectMenu::NP_MAX; i++)
		{
			NewProjectMenu menu = (NewProjectMenu)i;
			bool selected = m_CurrentMenu == menu;
			ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));
			if (ImGui::Selectable(NEWPROJECTMENUTOSTRING.at(menu), selected, 0, ImVec2(0.0f, 50.0f)))
			{
				m_CurrentMenu = menu;
			}
			ImGui::PopStyleVar();
		}
		ImGui::PopFont();

		ImGui::EndChild();
		ImGui::EndChild();
	}

	void NewProjectWindow::DrawRightPanel()
	{
		ImGui::SameLine();
		ImGui::BeginChild("RightPanel", ImVec2(), true);

		DrawHeader();

		ImGui::BeginChild("RightPanelBody", ImVec2(), true);
		switch (m_CurrentMenu)
		{
		case Glory::EditorLauncher::General:
			GeneralSettingsMenu();
			break;
		case Glory::EditorLauncher::Engine:
			EngineSettingsMenu();
			break;
		default:
			break;
		}

		ImGui::EndChild();
		ImGui::EndChild();
	}

	void NewProjectWindow::GeneralSettingsMenu()
	{
		ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		float maxItemSize = regionAvail.x - 200.0f;

#pragma region Editor Version

		ImGui::TextUnformatted("Editor Version:");
		std::string versionString = "Select Editor";

		if (m_SelectedEditorIndex != -1)
		{
			const EditorInfo& selectedEditor = EditorManager::GetEditorInfo(m_SelectedEditorIndex);
			selectedEditor.Version.GetVersionString(versionString);
		}

		size_t currentIndex = m_SelectedEditorIndex;
		ImGui::SameLine(regionAvail.x - maxItemSize);
		ImGui::SetNextItemWidth(maxItemSize);
		if (ImGui::BeginCombo("##EditorVersion", versionString.c_str()))
		{
			for (size_t i = 0; i < EditorManager::EditorCount(); i++)
			{
				const EditorInfo& editorInfo = EditorManager::GetEditorInfo(i);
				bool selected = m_SelectedEditorIndex == i;
				editorInfo.Version.GetVersionString(versionString);
				if (ImGui::Selectable(versionString.c_str(), selected))
				{
					m_SelectedEditorIndex = (int)i;
					TemplateManager::LoadTemplates(editorInfo);
					m_SelectedTemplate = TemplateManager::TemplateCount() > 0 ? 0 : -1;
				}
			}
			ImGui::EndCombo();
		}

		if (currentIndex != m_SelectedEditorIndex)
		{
			const EditorInfo& selectedEditor = EditorManager::GetEditorInfo(m_SelectedEditorIndex);
			ModuleManager::LoadModules(selectedEditor.RootPath);

			// TODO: Load default engine settings?
			m_EngineSettings = {};
			m_EngineSettings.ScriptingModules.push_back(0);
			m_EngineSettings.OptionalModules.push_back(0);
			m_EngineSettings.OptionalModules.push_back(2);
			m_EngineSettings.OptionalModules.push_back(1);
		}

#pragma endregion

		ImGui::Text("Project Name");
		ImGui::SameLine(regionAvail.x - maxItemSize);
		ImGui::SetNextItemWidth(maxItemSize);
		ImGui::InputText("##Project Name", m_ProjectNameText, 100);

		ImGui::Text("Project Path");
		ImGui::SameLine(regionAvail.x - maxItemSize);
		ImGui::SetNextItemWidth(400.0f);
		ImGui::InputText("##Project Path", m_PathText, 100);
		ImGui::SameLine();
		ImGui::TextUnformatted(m_ProjectFolder != "" ? m_ProjectFolder.c_str() : m_ProjectNameText);
		ImGui::SameLine();
		if (ImGui::Button("Browse", ImVec2(ImGui::GetContentRegionAvail().x - 8.0f, 0.0f)))
		{
			const char* filters[1] = { "*.gproj" };
			const char* path = tinyfd_saveFileDialog("Save Project", m_PathText, 1, filters, "Glorious Projects");

			if (path)
			{
				std::filesystem::path fullPath = path;
				std::filesystem::path fileName = fullPath.filename();
				fileName = fileName.replace_extension("");
				std::string fileNameString = fileName.string();
				strcpy(m_ProjectNameText, fileNameString.data());
				fullPath = fullPath.parent_path();
				std::filesystem::path::iterator last = fullPath.end();
				--last;
				std::filesystem::path folderName = *last;
				std::string folderNameString = folderName.string();
				m_ProjectFolder = "";
				if (folderNameString != fileName) m_ProjectFolder = folderNameString;
				m_BrowsingPath = fullPath.parent_path().string() + "\\";
				strcpy(m_PathText, m_BrowsingPath.data());
			}
		}

		bool exists = ProjectExists(m_PathText, m_ProjectNameText);
		bool tooShort = strlen(m_ProjectNameText) < MINPROJECTNAMELENGTH;

		if (exists)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Project already exists!");
		}

		if (tooShort)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Project name must be at least 3 characters long!");
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Template");

		const float templateSize = 128.0f;
		const float textHeight = ImGui::CalcTextSize("LABEL").y;
		const float windowMargin = ImGui::GetStyle().WindowPadding.y;
		ImGui::BeginChild("Templates", { 0.0f, templateSize + windowMargin * 2.0f });
		for (size_t i = 0; i < TemplateManager::TemplateCount(); i++)
		{
			ImGui::PushID((int)i);
			const ProjectTemplate* pTemplate = TemplateManager::GetTemplate(i);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::BeginChild("##template", { templateSize, templateSize }, false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::PopStyleVar(2);

			ImGui::PushFont(LauncherHub::IconFontHuge);

			const ImVec2 cursorPos = ImGui::GetCursorPos();
			if (ImGui::Selectable("##templateButton", m_SelectedTemplate == i, ImGuiSelectableFlags_AllowItemOverlap, { templateSize, templateSize }))
				m_SelectedTemplate = (int)i;

			const char* icon = TEMPLATE_ICONS.find(pTemplate->m_Name) != TEMPLATE_ICONS.end() ? TEMPLATE_ICONS.at(pTemplate->m_Name) : UNKNOWN_TEMPLATE_ICON;
			const ImVec2 iconSize = ImGui::CalcTextSize(icon);
			ImGui::SetCursorPos({ cursorPos.x + templateSize / 2.0f - iconSize.x / 2.0f, cursorPos.y + 20.0f });
			ImGui::TextUnformatted(icon);
			ImGui::PopFont();

			const ImVec2 textSize = ImGui::CalcTextSize(pTemplate->m_Name.data());

			ImGui::SetCursorPos({ cursorPos.x + templateSize / 2.0f - textSize.x / 2.0f, cursorPos.y + templateSize - textSize.y - 10.0f });
			ImGui::TextUnformatted(pTemplate->m_Name.data());
			ImGui::EndChild();

			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::EndChild();

		m_Valid = !exists && !tooShort;
	}

	void NewProjectWindow::EngineSettingsMenu()
	{
		bool validEditor = m_SelectedEditorIndex != -1;

		ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		float maxItemSize = regionAvail.x - 200.0f;

		ImGui::BeginDisabled(!validEditor);

		DrawModuleSelector("Window", ModuleType::MT_Window, m_EngineSettings.WindowModule, regionAvail.x - maxItemSize, maxItemSize);
		DrawModuleSelector("Graphics", ModuleType::MT_Graphics, m_EngineSettings.GraphicsModule, regionAvail.x - maxItemSize, maxItemSize);
		DrawModuleSelector("Renderer", ModuleType::MT_Renderer, m_EngineSettings.RenderModule, regionAvail.x - maxItemSize, maxItemSize);
		DrawModuleSelector("Input", ModuleType::MT_Input, m_EngineSettings.InputModule, regionAvail.x - maxItemSize, maxItemSize);

		DrawScriptingModulesArray(regionAvail, maxItemSize);
		DrawOptionalModulesArray(regionAvail, maxItemSize);

		ImGui::EndDisabled();

		if (!validEditor)
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Please select an Editor in the General tab!");
	}

	ProjectValidationResult NewProjectWindow::ValidateSettings()
	{
		if (m_SelectedEditorIndex < 0) return ProjectValidationResult::EVR_NoEditor;

		if (!m_Valid) return ProjectValidationResult::EVR_InvalidName;

		if (ProjectExists(m_PathText, m_ProjectNameText)) return ProjectValidationResult::EVR_InvalidName;
		if (strlen(m_ProjectNameText) < MINPROJECTNAMELENGTH) return ProjectValidationResult::EVR_AlreadyExists;

		if (!ValidateModule(ModuleType::MT_Window, m_EngineSettings.WindowModule)) return ProjectValidationResult::EVR_MissingModules;
		if (!ValidateModule(ModuleType::MT_Graphics, m_EngineSettings.GraphicsModule)) return ProjectValidationResult::EVR_MissingModules;
		if (!ValidateModule(ModuleType::MT_Renderer, m_EngineSettings.RenderModule)) return ProjectValidationResult::EVR_MissingModules;

		for (size_t i = 0; i < m_EngineSettings.OptionalModules.size(); i++)
		{
			if (!ValidateModule(ModuleType::MT_Other, m_EngineSettings.OptionalModules[i])) return ProjectValidationResult::EVR_MissingModules;
		}

		for (size_t i = 0; i < m_EngineSettings.ScriptingModules.size(); i++)
		{
			if (!ValidateModule(ModuleType::MT_Scripting, m_EngineSettings.ScriptingModules[i])) return ProjectValidationResult::EVR_MissingModules;
		}

		std::vector<int> sorted = m_EngineSettings.ScriptingModules;
		std::sort(sorted.begin(), sorted.end());
		if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) return ProjectValidationResult::EVR_DuplicateScriptingModules;

		sorted = m_EngineSettings.OptionalModules;
		std::sort(sorted.begin(), sorted.end());
		if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) return ProjectValidationResult::EVR_DuplicateModules;

		if (m_SelectedTemplate == -1 || m_SelectedTemplate >= TemplateManager::TemplateCount())
			return ProjectValidationResult::EVR_NoTemplate;

		return ProjectValidationResult::EVR_Success;
	}

	bool NewProjectWindow::ValidateModule(const ModuleType& moduleType, const int& currentIndex)
	{
		if (currentIndex < 0) return false;

		std::vector<ModuleMetaData*> modules;
		ModuleManager::GetModulesOfType(moduleType, modules);

		return currentIndex < modules.size();
	}

	bool NewProjectWindow::DrawModuleSelector(const char* name, const ModuleType& moduleType, int& currentIndex, float posX, float maxSize, bool showErrorOnInvalid)
	{
		std::vector<ModuleMetaData*> modules;
		ModuleManager::GetModulesOfType(moduleType, modules);

		bool valid = currentIndex < modules.size() && currentIndex > -1;

		ImGui::Text(name);

		ImGui::SameLine(posX);
		ImGui::SetNextItemWidth(maxSize);
		std::string label = "##" + std::string(name);
		if (ImGui::BeginCombo(label.c_str(), valid ? modules[currentIndex]->Name().c_str() : "None"))
		{
			if (ImGui::Selectable("None", currentIndex == -1))
			{
				currentIndex = -1;
			}

			for (size_t i = 0; i < modules.size(); i++)
			{
				std::string name = modules[i]->Name();
				bool selected = currentIndex == i;
				if (ImGui::Selectable(name.c_str(), selected))
				{
					currentIndex = (int)i;
				}
			}

			ImGui::EndCombo();
		}

		if (!valid && showErrorOnInvalid)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "This is a required module!");
		}

		return valid;
	}

	void NewProjectWindow::DrawOptionalModulesArray(ImVec2 regionAvail, float maxItemSize)
	{
		ImGui::Spacing();
		ImGui::Text("Optional Modules");
		ImGui::SameLine(regionAvail.x - 25.0f);
		if (ImGui::Button("+##AddOptionalModule", ImVec2(25.0f, 25.0f)))
		{
			m_EngineSettings.OptionalModules.push_back(0);
		}

		int toRemoveIndex = -1;
		for (size_t i = 0; i < m_EngineSettings.OptionalModules.size(); i++)
		{
			std::string label = "Optional Module " + std::to_string(i + 1);
			bool valid = DrawModuleSelector(label.c_str(), ModuleType::MT_Other, m_EngineSettings.OptionalModules[i], regionAvail.x - maxItemSize, maxItemSize - 25.0f - 8.0f, false);

			label = "-##RemoveModule" + std::to_string(i);
			ImGui::SameLine();
			if (ImGui::Button(label.c_str(), ImVec2(25.0f, 25.0f)))
			{
				toRemoveIndex = (int)i;
			}

			if (!valid)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "This cannot be None!");
			}
		}

		if (toRemoveIndex >= 0) m_EngineSettings.OptionalModules.erase(m_EngineSettings.OptionalModules.begin() + toRemoveIndex);

		std::vector<int> sorted = m_EngineSettings.OptionalModules;
		std::sort(sorted.begin(), sorted.end());
		if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end())
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Cannot have multiple instances of the same Module!");
		}
	}

	void NewProjectWindow::DrawScriptingModulesArray(ImVec2 regionAvail, float maxItemSize)
	{
		ImGui::Spacing();
		ImGui::Text("Scripting Modules");
		ImGui::SameLine(regionAvail.x - 25.0f);
		if (ImGui::Button("+##AddScriptingModule", ImVec2(25.0f, 25.0f)))
		{
			m_EngineSettings.ScriptingModules.push_back(0);
		}

		int toRemoveIndex = -1;
		for (size_t i = 0; i < m_EngineSettings.ScriptingModules.size(); i++)
		{
			std::string label = "Optional Module " + std::to_string(i + 1);
			bool valid = DrawModuleSelector(label.c_str(), ModuleType::MT_Scripting, m_EngineSettings.ScriptingModules[i], regionAvail.x - maxItemSize, maxItemSize - 25.0f - 8.0f, false);

			label = "-##RemoveModule" + std::to_string(i);
			ImGui::SameLine();
			if (ImGui::Button(label.c_str(), ImVec2(25.0f, 25.0f)))
			{
				toRemoveIndex = (int)i;
			}

			if (!valid)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "This cannot be None!");
			}
		}

		if (toRemoveIndex >= 0) m_EngineSettings.ScriptingModules.erase(m_EngineSettings.ScriptingModules.begin() + toRemoveIndex);

		std::vector<int> sorted = m_EngineSettings.ScriptingModules;
		std::sort(sorted.begin(), sorted.end());
		if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end())
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Cannot have multiple instances of the same Module!");
		}
	}

	bool NewProjectWindow::ProjectExists(const std::string& path, const std::string& name)
	{
		return std::filesystem::exists(GetProjectPath(path, name));
	}

	std::filesystem::path NewProjectWindow::GetProjectPath(const std::string& path, const std::string& name)
	{
		std::filesystem::path projectPath(path);
		projectPath.append(m_ProjectFolder == "" ? name : m_ProjectFolder);
		projectPath.append(name).replace_extension(".gproj");
		return projectPath;
	}
}
