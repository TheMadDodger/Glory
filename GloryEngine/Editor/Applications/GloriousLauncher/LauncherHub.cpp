#include "pch.h"
#include "LauncherHub.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "ProjectManager.h"
#include "ProjectLock.h"
#include "EditorManager.h"

#include <tinyfiledialogs.h>
#include <GloryAPI.h>
#include <About.h>

namespace Glory::EditorLauncher
{
    constexpr char* MenuNames[] = {
        "Projects",
        "Installs",
        "Support",
        "About",
    };

    ImFont* LauncherHub::DefaultFont = nullptr;
    ImFont* LauncherHub::BoldLargeFont = nullptr;
    ImFont* LauncherHub::IconFontHuge = nullptr;

	LauncherHub::LauncherHub(ImGuiImpl* pHubWindow) : m_pImGuiImpl(pHubWindow), m_OpenErrorPopup(false), m_OpenNewProjectPopup(false), m_OpenProjectOpenError(false), m_OpenMissingEditorError(false), m_ProjectFolder("")
	{
	}

	LauncherHub::~LauncherHub()
	{
	}

	void LauncherHub::Run()
	{
        GloryAPI::Initialize();

        ImGuiIO& io = ImGui::GetIO();
        DefaultFont = io.Fonts->AddFontFromFileTTF("./Fonts/PT_Sans/PTSans-Regular.ttf", 18.0f);
        BoldLargeFont = io.Fonts->AddFontFromFileTTF("./Fonts/PT_Sans/PTSans-Bold.ttf", 32.0f);
        io.FontDefault = DefaultFont;

        static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        static const ImWchar brandIconRanges[] = { ICON_MIN_FAB, ICON_MAX_FAB, 0 };

        ImFontConfig config;
        config.DstFont = DefaultFont;
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphMinAdvanceX = 14.0f;
        ImFont* pIconFont = io.Fonts->AddFontFromFileTTF("./Fonts/FA/" FONT_ICON_FILE_NAME_FAS, 14.0f, &config, iconRanges);
        ImFont* pIconBrandsFont = io.Fonts->AddFontFromFileTTF("./Fonts/FA/" FONT_ICON_FILE_NAME_FAB, 14.0f, &config, brandIconRanges);

        IconFontHuge = io.Fonts->AddFontFromFileTTF("./Fonts/FA/" FONT_ICON_FILE_NAME_FAS, 64.0f, NULL, iconRanges);

        ProjectManager::Load();

        /* Theme */
        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);

        // Headers
        colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

        // Buttons
        colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

        // Frames BG
        colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.3805f, 0.381f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.2805f, 0.281f, 1.0f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

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

        GloryAPI::Cleanup();
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

        //if (ImGui::Button(ICON_FA_GEAR, ImVec2(contentRegionAvail.y, contentRegionAvail.y)))
        //{
        //
        //}
        ImGui::EndChild();
        ImGui::BeginChild("LeftPanelBody", ImVec2(0.0f, 0.0f), false);

        for (size_t i = 0; i < HubMenu::MAX; i++)
        {
            HubMenu menu = (HubMenu)i;
            bool selected = m_CurrentMenu == menu;
            ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));
            if (ImGui::Selectable(MenuNames[i], selected, 0, ImVec2(0.0f, 50.0f)))
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
        case Glory::EditorLauncher::Support:
            DrawSupport();
            break;
        case Glory::EditorLauncher::About:
            DrawAbout();
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
            const char* filters[1] = { "*.gproj" };
            const char* path = tinyfd_openFileDialog("Open Project", m_DefaultProjectsFolder.c_str(), 1, filters, "Glorious Project", false);
            if (path)
            {
                if (!std::filesystem::exists(path))
                {
                    return;
                }
                ProjectManager::AddProject(path);
            }
        }
        ImGui::SameLine(regionWidth - buttonWidth + 8.0f);
        if (ImGui::Button("NEW", ImVec2(buttonWidth, 0.0f)))
        {
            m_BrowsingPath = m_DefaultProjectsFolder;
            strcpy(m_PathText, m_BrowsingPath.data());
            m_pNewProjectWindow.Open();
        }

        static ImGuiTableFlags flags =
            //ImGuiTableFlags_Resizable
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_SizingFixedFit;

        const ImVec2 availableRegion = ImGui::GetContentRegionAvail();
        if (ImGui::BeginTable("Project Hub", 3, flags, availableRegion))
        {
            ImGui::TableSetupColumn("Project Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 1.0f, 0);
            ImGui::TableSetupColumn("Modified", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.2f, 1);
            ImGui::TableSetupColumn("Editor Version", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.2f, 2);

            ImGui::TableHeadersRow();

            for (size_t row_n = 0; row_n < ProjectManager::ProjectCount(); row_n++)
            {
                const Project* item = ProjectManager::GetProject(row_n);

                bool editorInstalled = EditorManager::IsInstalled(item->SelectedVersion);

                ImGui::PushID((int)row_n);
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
                    std::string versionString;
                    item->SelectedVersion.GetVersionString(versionString);
                    if (ImGui::BeginCombo("##CurrentVersion", versionString.c_str()))
                    {
                        for (size_t i = 0; i < EditorManager::EditorCount(); i++)
                        {
                            const EditorInfo& editorInfo = EditorManager::GetEditorInfo(i);
                            const bool selected = Version::Compare(editorInfo.Version, item->SelectedVersion) == 0;
                            std::string otherVersionString;
                            editorInfo.Version.GetVersionString(otherVersionString);
                            if (ImGui::Selectable(otherVersionString.c_str(), selected))
                            {
                                item->SelectedVersion = editorInfo.Version;
                            }
                        }
                        if (!EditorManager::IsInstalled(item->Version))
                        {
                            bool selected = Version::Compare(item->Version, item->SelectedVersion) == 0;
                            std::string missingEditor;
                            item->Version.GetVersionString(missingEditor);
                            missingEditor += " !";
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
        ImGui::BeginDisabled(true);
        if (ImGui::Button("Locate", ImVec2(buttonWidth, 0.0f)))
        {
            const char* filters[1] = { "GloryEditor.dll" };
            const char* path = tinyfd_openFileDialog("Open Editor", ".\\", 1, filters, "GloryEditor.dll", false);

            if (path)
            {
                if (!std::filesystem::exists(path))
                {
                    return;
                }
            }
        }
        ImGui::SameLine(regionWidth - buttonWidth + 8.0f);
        if (ImGui::Button("Install Editor", ImVec2(buttonWidth, 0.0f)))
        {

        }
        ImGui::EndDisabled();

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

                ImGui::PushID((int)row_n);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);

                ImGui::TableSetColumnIndex(0);
                ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

                std::string editorString("Version ");
                std::string versionString;
                editorInfo.Version.GetVersionString(versionString);
                editorString += versionString;
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

    void LauncherHub::DrawSupport()
    {
        ImGui::Text("Support");
        ImGui::Separator();
        ImGui::Text("The following links may provide you with help");

        const float width = ImGui::GetContentRegionAvail().x;
        if (ImGui::Button("glory-engine.com", { width , 0.0f }))
        {
            system("start \"\" \"https://glory-engine.com\"");
        }
        if (ImGui::Button("docs.glory-engine.com/", { width , 0.0f }))
        {
            system("start \"\" \"https://docs.glory-engine.com\"");
        }
        ImGui::Text("You can also join our discord and get help from our community");
        if (ImGui::Button("Join Discord", { width , 0.0f }))
        {
            system("start \"\" \"https://discord.gg/e8Tzqbb\"");
        }
    }

    void LauncherHub::DrawAbout()
    {
        ImGui::TextUnformatted("About");
        ImGui::Separator();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
        ImGui::BeginChild("AboutText");
        ImGui::PopStyleVar();
        ImGui::TextUnformatted(AboutText);
        ImGui::EndChild();

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
