#include "pch.h"
#include "LauncherHub.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "ImFileDialog.h"
#include "ProjectManager.h"
#include "ProjectLock.h"
#include "EditorManager.h"

namespace Glory::EditorLauncher
{
    std::map<HubMenu, const char*> MENUTOSTRING = {
        { HubMenu::ProjectList, "Projects" },
        { HubMenu::EditorList, "Installs" },
    };

    ImFont* LauncherHub::DefaultFont = nullptr;
    ImFont* LauncherHub::BoldLargeFont = nullptr;

    std::function<void(const std::string&)> LauncherHub::FileBrowserCallback;

	LauncherHub::LauncherHub(ImGuiImpl* pHubWindow) : m_pImGuiImpl(pHubWindow), m_OpenErrorPopup(false), m_OpenNewProjectPopup(false), m_OpenProjectOpenError(false), m_OpenMissingEditorError(false), m_ProjectFolder("")
	{
	}

	LauncherHub::~LauncherHub()
	{
	}

	void LauncherHub::Run()
	{
        ImGuiIO& io = ImGui::GetIO();
        DefaultFont = io.Fonts->AddFontFromFileTTF("./Fonts/PT_Sans/PTSans-Regular.ttf", 18.0f);
        BoldLargeFont = io.Fonts->AddFontFromFileTTF("./Fonts/PT_Sans/PTSans-Bold.ttf", 32.0f);
        io.FontDefault = DefaultFont;

        InitializeFileDialog();

        ProjectManager::Load();

        // Main loop
        bool done = false;
        while (!done)
        {
            HubWindow* pHubWindow = m_pImGuiImpl->GetHubWindow();
            done = pHubWindow->PollEvents();
            m_pImGuiImpl->NewFrame();
            Draw();
            m_pImGuiImpl->Render();
            pHubWindow->Swap();
        }

        ProjectManager::Save();
	}

    void LauncherHub::InitializeFileDialog()
    {
        ifd::FileDialog::Instance().CreateTexture = [](const std::string& path, uint8_t* data, int w, int h, char fmt) -> void* {
            GLuint tex;

            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);

            return (void*)tex;
        };
        ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
            GLuint texID = (GLuint)tex;
            glDeleteTextures(1, &texID);
        };
    }

    void LauncherHub::Draw()
    {
        //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
        ImGui::Begin("Hub Window", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);

        if (!m_pNewProjectWindow.Draw())
        {
            DrawLeftPanel();
            DrawRightPanel();
        }

        ImGui::End();


        DrawPopups();
    }

    void LauncherHub::DrawFileDialog()
    {
        if (ifd::FileDialog::Instance().IsDone(FILEDIALOG_ID)) {
            if (ifd::FileDialog::Instance().HasResult()) {
                std::string res = ifd::FileDialog::Instance().GetResult().u8string();
                printf("OPEN[%s]\n", res.c_str());
                FileBrowserCallback(res);
            }
            ifd::FileDialog::Instance().Close();
        }
    }

    std::string LauncherHub::FormatTimestamp(long long timestamp)
    {
        std::time_t nowtt(timestamp);
        std::tm* nowtm = std::gmtime(&nowtt);
        std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(nowtt);
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::system_clock::duration duration = now - timePoint;
        std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

        std::stringstream stream;

        long long secondsCount = seconds.count();
        long long minutesCount = secondsCount / 60;
        long long hourCount = minutesCount / 60;
        long long dayCount = hourCount / 24;
        long long yearCount = dayCount / 365;

        if (yearCount >= 1) stream << std::put_time(nowtm, "%d-%m-%Y");
        else if (dayCount > 1) stream << dayCount << " days ago";
        else if (dayCount == 1) stream << "a day ago";
        else if (hourCount > 1) stream << hourCount << " hours ago";
        else if (hourCount == 1) stream << "an hour ago";
        else if (minutesCount > 1) stream << minutesCount << " minutes ago";
        else if (minutesCount == 1) stream << "a minute ago";
        else if (secondsCount == 1) stream << "a second ago";
        else stream << secondsCount << " seconds ago";

        return stream.str();
    }

    void LauncherHub::OnProjectNotFoundGui()
    {
        ImGui::Text("The project could not be found, would you like to remove it from the list?");
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.x /= 2.0f;
        size.x -= 4.0f;
        size.y = ImGui::GetTextLineHeightWithSpacing();

        if (ImGui::Button("Yes", size))
        {
            ProjectManager::RemoveProjectAt(m_RemovingIndex);
            m_RemovingIndex = -1;
            ImGui::CloseCurrentPopup();
            m_OpenErrorPopup = false;
            ProjectManager::Save();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", size))
        {
            ImGui::CloseCurrentPopup();
            m_OpenErrorPopup = false;
        }
    }

    void LauncherHub::OnNewProjectPopupGui()
    {
        float x = ImGui::CalcTextSize("Project Name").x + 20.0f;

        ImGui::Text("New Project");
        ImGui::Spacing();
        ImGui::Text("Project Name");
        ImGui::SameLine(x);
        ImGui::SetNextItemWidth(600.0f);
        ImGui::InputText("##Project Name", m_ProjectNameText, 100);
        ImGui::Spacing();
        ImGui::Text("Project Path");
        ImGui::SameLine(x);
        ImGui::SetNextItemWidth(350.0f);
        ImGui::InputText("##Project Path", m_PathText, 100);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::TextUnformatted(m_ProjectFolder != "" ? m_ProjectFolder.c_str() : m_ProjectNameText);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        if (ImGui::Button("Browse"))
        {
            FileBrowserCallback = [&](const std::string& path)
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
                m_BrowsingPath = fullPath.parent_path().string();
                strcpy(m_PathText, m_BrowsingPath.data());
            };
            ifd::FileDialog::Instance().Save(FILEDIALOG_ID, "Save project", "Project file (*.gproj){.gproj},.*", m_PathText);
        }

        ImGui::Spacing();

        bool exists = ProjectExists(m_PathText, m_ProjectNameText);
        bool tooShort = strlen(m_ProjectNameText) < 3;

        if (exists)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Project already exists!");
        }

        if (tooShort)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Project name must be at least 3 characters long!");
        }

        bool valid = !exists && !tooShort;

        ImGui::BeginDisabled(!valid);
        if (ImGui::Button("Create Project", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)) && valid)
        {
            ProjectCreateSettings createSettings{};
            createSettings.Name = m_ProjectNameText;
            createSettings.Path = GetProjectPath(m_PathText, m_ProjectNameText).string();
            // Other settings!

            ProjectManager::CreateProject(createSettings);

            ImGui::CloseCurrentPopup();
            m_OpenNewProjectPopup = false;
        }
        ImGui::EndDisabled();

        if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
            m_OpenNewProjectPopup = false;
        }

        DrawFileDialog();
    }

    bool LauncherHub::ProjectExists(const std::string& path, const std::string& name)
    {
        return std::filesystem::exists(GetProjectPath(path, name));
    }
    
    std::filesystem::path LauncherHub::GetProjectPath(const std::string& path, const std::string& name)
    {
        std::filesystem::path projectPath(path);
        projectPath.append(m_ProjectFolder == "" ? name : m_ProjectFolder);
        projectPath.append(name).replace_extension(".gproj");
        return projectPath;
    }

    void LauncherHub::DrawLeftPanel()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushFont(BoldLargeFont);
        ImGui::BeginChild("LeftPanel", ImVec2(250.0f, 0.0f), true);
        ImGui::BeginChild("LeftPanelHeader", ImVec2(0.0f, 50.0f), false);

        ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
        float size = BoldLargeFont->FontSize;
        float cursorPosY = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (size / 2.0f) + (contentRegionAvail.y / 2.0f));
        ImGui::TextUnformatted("Glory");
        ImGui::SetCursorPosY(cursorPosY);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + contentRegionAvail.x - contentRegionAvail.y);

        if (ImGui::Button("S", ImVec2(contentRegionAvail.y, contentRegionAvail.y)))
        {

        }
        ImGui::EndChild();
        ImGui::BeginChild("LeftPanelBody", ImVec2(0.0f, 0.0f), false);

        for (size_t i = 0; i < HubMenu::MAX; i++)
        {
            HubMenu menu = (HubMenu)i;
            bool selected = m_CurrentMenu == menu;
            ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));
            if (ImGui::Selectable(MENUTOSTRING[menu], selected, 0, ImVec2(0.0f, 50.0f)))
            {
                m_CurrentMenu = menu;
            }
            ImGui::PopStyleVar();
        }
        ImGui::PopFont();

        ImGui::EndChild();
        ImGui::EndChild();
    }

    void LauncherHub::DrawRightPanel()
    {
        ImGui::SameLine();
        ImGui::BeginChild("RightPanel", ImVec2(), true);

        switch (m_CurrentMenu)
        {
        case Glory::EditorLauncher::ProjectList:
            DrawProjectList();
            break;
        case Glory::EditorLauncher::EditorList:
            DrawInstalledEditorsList();
            break;
        case Glory::EditorLauncher::MAX:
            break;
        default:
            break;
        }
        ImGui::EndChild();
    }

    void LauncherHub::DrawProjectList()
    {
        float regionWidth = ImGui::GetContentRegionAvail().x;

        float buttonWidth = 150.0f;

        ImGui::Text("Projects");
        ImGui::SameLine(regionWidth - buttonWidth * 2.0f);
        if (ImGui::Button("ADD", ImVec2(buttonWidth, 0.0f)))
        {
            FileBrowserCallback = [&](const std::string& path)
            {
                if (!std::filesystem::exists(path))
                {
                    return;
                }
                ProjectManager::AddProject(path);
            };
            ifd::FileDialog::Instance().Open(FILEDIALOG_ID, "Open a project", "Project file (*.gproj){.gproj},.*", false);//, m_DefaultProjectsFolder);
        }
        ImGui::SameLine(regionWidth - buttonWidth + 8.0f);
        if (ImGui::Button("NEW", ImVec2(buttonWidth, 0.0f)))
        {
            //m_BrowsingPath = m_DefaultProjectsFolder;
            //strcpy(m_PathText, m_BrowsingPath.data());
            //m_OpenNewProjectPopup = true;

            m_pNewProjectWindow.Open();
        }

        static ImGuiTableFlags flags =
            //ImGuiTableFlags_Resizable
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_SizingFixedFit;

        if (ImGui::BeginTable("Project Hub", 3, flags, ImVec2(0, 0), 0.0f))
        {
            ImGui::TableSetupColumn("Project Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 0);
            ImGui::TableSetupColumn("Modified", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 1);
            ImGui::TableSetupColumn("Editor Version", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 200.0f, 2);
            //ImGui::TableSetupColumn("Project Path", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 1);

            ImGui::TableHeadersRow();

            //ImGui::PushButtonRepeat(true);

            for (size_t row_n = 0; row_n < ProjectManager::ProjectCount(); row_n++)
            {
                const Project* item = ProjectManager::GetProject(row_n);

                bool editorInstalled = EditorManager::IsInstalled(item->SelectedVersion);

                ImGui::PushID(row_n);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);

                ImGui::TableSetColumnIndex(0);
                ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

                std::string projectNameString(item->Name);
                projectNameString += '\n';
                projectNameString += "------------------------\n";
                projectNameString += item->Path;

                if (ImGui::Selectable(projectNameString.data(), false, selectable_flags, ImVec2(0, 0.0f)))
                {
                    if (!std::filesystem::exists(item->Path))
                    {
                        m_RemovingIndex = row_n;
                        m_OpenErrorPopup = true;
                    }
                    else
                    {
                        if (!editorInstalled)
                        {
                            m_OpenMissingEditorError = true;
                        }
                        else
                        {
                            ProjectLock lock(item->Path);
                            if (!lock.CanLock())
                            {
                                // Open a popup
                                m_OpenProjectOpenError = true;
                            }
                            else ProjectManager::OpenProject(row_n);
                        }
                    }
                }

                if (ImGui::TableSetColumnIndex(1))
                {
                    long long timestamp = item->LastEdit;
                    std::string lastEdit = FormatTimestamp(timestamp);
                    ImGui::TextUnformatted(lastEdit.c_str());
                }

                if (ImGui::TableSetColumnIndex(2))
                {
                    if (ImGui::BeginCombo("##CurrentVersion", item->SelectedVersion.GetVersionString().c_str()))
                    {
                        for (size_t i = 0; i < EditorManager::EditorCount(); i++)
                        {
                            const EditorInfo& editorInfo = EditorManager::GetEditorInfo(i);
                            bool selected = editorInfo.Version.HardCompare(item->SelectedVersion) == 0;
                            if (ImGui::Selectable(editorInfo.Version.GetVersionString().c_str(), selected))
                            {
                                item->SelectedVersion = editorInfo.Version;
                            }
                        }
                        if (!EditorManager::IsInstalled(item->Version))
                        {
                            bool selected = item->Version.HardCompare(item->SelectedVersion) == 0;
                            std::string missingEditor = item->Version.GetVersionString() + " !";
                            if (ImGui::Selectable(missingEditor.c_str(), selected))
                            {
                                item->SelectedVersion = item->Version;
                            }
                        }
                        ImGui::EndCombo();
                    }

                    if (!editorInstalled)
                    {
                        ImGui::SameLine();
                        ImGui::TextUnformatted("!");
                    }
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }

    void LauncherHub::DrawInstalledEditorsList()
    {
        float regionWidth = ImGui::GetContentRegionAvail().x;

        float buttonWidth = 150.0f;

        ImGui::Text("Installs");
        ImGui::SameLine(regionWidth - buttonWidth * 2.0f);
        if (ImGui::Button("Locate", ImVec2(buttonWidth, 0.0f)))
        {
            FileBrowserCallback = [&](const std::string& path)
            {
                if (!std::filesystem::exists(path))
                {
                    return;
                }
            };
            ifd::FileDialog::Instance().Open(FILEDIALOG_ID, "Open GloryEditor.dll", "GloryEditor (GloryEditor.dll){GloryEditor.gproj}", false);//, m_DefaultProjectsFolder);
        }
        ImGui::SameLine(regionWidth - buttonWidth + 8.0f);
        if (ImGui::Button("Install Editor", ImVec2(buttonWidth, 0.0f)))
        {
            
        }

        static ImGuiTableFlags flags =
            //ImGuiTableFlags_Resizable
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_SizingFixedFit;

        if (ImGui::BeginTable("Installed Editors", 1, flags, ImVec2(0, 0), 0.0f))
        {
            //ImGui::TableSetupColumn("Editor", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 0);
            //ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 1);
            //ImGui::TableSetupColumn("Project Path", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 1);

            //ImGui::TableHeadersRow();

            //ImGui::PushButtonRepeat(true);

            for (size_t row_n = 0; row_n < EditorManager::EditorCount(); row_n++)
            {
                const EditorInfo& editorInfo = EditorManager::GetEditorInfo(row_n);

                ImGui::PushID(row_n);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);

                ImGui::TableSetColumnIndex(0);
                ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

                std::string editorString("Version ");
                editorString += editorInfo.Version.GetVersionString();
                editorString += '\n';
                editorString += "-----------------------------------------------------------------\n";
                editorString += std::filesystem::absolute(editorInfo.RootPath).string();

                if (ImGui::Selectable(editorString.data(), false, selectable_flags, ImVec2(0, 0.0f)))
                {
                    
                }

                //if (ImGui::TableSetColumnIndex(1))
                //{
                //    if (ImGui::Button("S"))
                //    {
                //
                //    }
                //}

                ImGui::PopID();
            }
            ImGui::EndTable();
        }


        for (size_t i = 0; i < EditorManager::EditorCount(); i++)
        {
            const EditorInfo& editorInfo = EditorManager::GetEditorInfo(i);
            
        }
    }

    void LauncherHub::DrawPopups()
    {
        if (m_OpenErrorPopup) ImGui::OpenPopup("Project Not Found");
        ImGui::SetWindowSize(ImVec2(0.0f, 0.0f));
        if (ImGui::BeginPopupModal("Project Not Found", &m_OpenErrorPopup, ImGuiWindowFlags_AlwaysAutoResize))
        {
            OnProjectNotFoundGui();
            ImGui::EndPopup();
        }

        if (m_OpenNewProjectPopup) ImGui::OpenPopup("New Project");

        ImGui::SetWindowSize(ImVec2(0.0f, 0.0f));
        if (ImGui::BeginPopupModal("New Project", &m_OpenNewProjectPopup, ImGuiWindowFlags_AlwaysAutoResize))
        {
            OnNewProjectPopupGui();
            ImGui::EndPopup();
        }
        else DrawFileDialog();

        if (m_OpenProjectOpenError) ImGui::OpenPopup("Error Opening Project");
        HubWindow* pHubWindow = m_pImGuiImpl->GetHubWindow();

        int w, h;
        SDL_GetWindowSize(pHubWindow->GetSDLWindow(), &w, &h);
        ImGui::SetNextWindowPos(ImVec2(w / 2.0f, h / 2.0f), ImGuiCond_Always, ImVec2(.5f, .5f));
        if (ImGui::BeginPopupModal("Error Opening Project", &m_OpenProjectOpenError, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushTextWrapPos(300.0f);
            ImGui::TextWrapped("Could not open the project because it is already opened by another Editor!");
            ImGui::PopTextWrapPos();
            float width = ImGui::GetContentRegionAvail().x;
            if (ImGui::Button("OK", ImVec2(width, 0.0f)))
            {
                ImGui::CloseCurrentPopup();
                m_OpenProjectOpenError = false;
            }
            ImGui::EndPopup();
        }
        
        if (m_OpenMissingEditorError) ImGui::OpenPopup("Missing Editor");
        SDL_GetWindowSize(pHubWindow->GetSDLWindow(), &w, &h);
        ImGui::SetNextWindowPos(ImVec2(w / 2.0f, h / 2.0f), ImGuiCond_Always, ImVec2(.5f, .5f));
        if (ImGui::BeginPopupModal("Missing Editor", &m_OpenMissingEditorError, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushTextWrapPos(300.0f);
            ImGui::TextWrapped("Could not open the project because you do not have the correct editor installed!\nSelect another editor from the drop down or install the correct editor.");
            ImGui::PopTextWrapPos();
            float width = ImGui::GetContentRegionAvail().x;
            if (ImGui::Button("OK", ImVec2(width, 0.0f)))
            {
                ImGui::CloseCurrentPopup();
                m_OpenMissingEditorError = false;
            }
            ImGui::EndPopup();
        }
    }
}
