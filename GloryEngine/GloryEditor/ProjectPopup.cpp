#include <imgui.h>
#include <filesystem>
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include "ProjectPopup.h"
#include "ProjectSpace.h"

namespace Glory::Editor
{
    GLORY_EDITOR_API ProjectPopup::ProjectPopup()
        : m_Open(false), m_OpenErrorPopup(false), m_OpenFileDialogPopup(false), m_OpenNewProjectPopup(false),
        m_Width(800.0f), m_Height(600.0f)
	{
        wchar_t* p;
        if (S_OK != SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &p))
        {
            m_DefaultProjectsFolder = "C:/";
            return;
        }

        std::filesystem::path result = p;
        CoTaskMemFree(p);

        result.append("Glorious\\Projects");

        m_DefaultProjectsFolder = result.string();

        if (!std::filesystem::exists(m_DefaultProjectsFolder))
            std::filesystem::create_directories(m_DefaultProjectsFolder);
	}

    GLORY_EDITOR_API ProjectPopup::~ProjectPopup()
	{
	}

    GLORY_EDITOR_API void ProjectPopup::Initialize()
    {
        wchar_t* p;
        if (S_OK != SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &p))
        {
            m_DefaultProjectsFolder = "C:/";
            return;
        }

        std::filesystem::path result = p;
        CoTaskMemFree(p);

        result.append("Glorious\\Projects");

        m_DefaultProjectsFolder = result.string();

        if (!std::filesystem::exists(m_DefaultProjectsFolder))
            std::filesystem::create_directories(m_DefaultProjectsFolder);

        Load();
    }

    GLORY_EDITOR_API void ProjectPopup::Load()
    {
        m_Projects.clear();
        std::filesystem::path path("Projects.list");
        if (!std::filesystem::exists(path)) return;
        YAML::Node node = YAML::LoadFile(path.string());
        for (size_t i = 0; i < node.size(); i++)
        {
            YAML::Node element = node[i];
            Project project;
            project.Name = element["ProjectName"].as<std::string>();
            project.Path = element["ProjectPath"].as<std::string>();
            m_Projects.push_back(project);
        }
    }

    GLORY_EDITOR_API void ProjectPopup::Save()
    {
        std::filesystem::path path("Projects.list");
        YAML::Emitter out;

        out << YAML::BeginSeq;
        for (size_t i = 0; i < m_Projects.size(); i++)
        {
            Project* pProject = &m_Projects[i];
            out << YAML::BeginMap;
            out << YAML::Key << "ProjectName";
            out << YAML::Value << pProject->Name;
            out << YAML::Key << "ProjectPath";
            out << YAML::Value << pProject->Path;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        std::ofstream outStream(path);
        outStream << out.c_str();
        outStream.close();

        Load();
    }

    GLORY_EDITOR_API void ProjectPopup::Open()
	{
		m_Open = true;
	}

    GLORY_EDITOR_API void ProjectPopup::OnGui()
	{
		if (m_Open)
			ImGui::OpenPopup("Project Hub");

        ImGui::SetNextWindowSize(ImVec2(m_Width, m_Height));
        
        auto& style = ImGui::GetStyle();
        ImVec2 minWindowSize = style.WindowMinSize;
        style.WindowMinSize = ImVec2(600.0f, 400.0f);

        if (ImGui::BeginPopupModal("Project Hub", &m_Open, ImGuiWindowFlags_NoTitleBar))
        {
            OnHubGui();
            ImGui::EndPopup();
        }

        style.WindowMinSize = minWindowSize;
	}

    GLORY_EDITOR_API void ProjectPopup::OnHubGui()
    {
        ImVec2 size = ImGui::GetWindowSize();
        m_Width = size.x;
        m_Height = size.y;

        float regionWidth = ImGui::GetContentRegionAvail().x;

        float buttonWidth = 150.0f;

        ImGui::Text("Projects");
        ImGui::SameLine(regionWidth - buttonWidth * 2.0f);
        if (ImGui::Button("ADD", ImVec2(buttonWidth, 0.0f)))
        {
            m_BrowsingPath = m_DefaultProjectsFolder;
            strcpy(m_PathText, m_BrowsingPath.data());
            m_OpenFileDialogPopup = true;
        }
        ImGui::SameLine(regionWidth - buttonWidth + 8.0f);
        if (ImGui::Button("NEW", ImVec2(buttonWidth, 0.0f)))
        {
            m_BrowsingPath = m_DefaultProjectsFolder;
            strcpy(m_PathText, m_BrowsingPath.data());
            m_OpenNewProjectPopup = true;
        }

        static ImGuiTableFlags flags =
            //ImGuiTableFlags_Resizable
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_SizingFixedFit;

        if (ImGui::BeginTable("Project Hub", 1, flags, ImVec2(0, 0), 0.0f))
        {
            ImGui::TableSetupColumn("Project Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 0);
            //ImGui::TableSetupColumn("Project Date", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 1);
            //ImGui::TableSetupColumn("Project Path", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 1);

            ImGui::TableHeadersRow();

            //ImGui::PushButtonRepeat(true);

            for (size_t row_n = 0; row_n < m_Projects.size(); row_n++)
            {
                Project* item = &m_Projects[row_n];

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
                    if (!ProjectSpace::ProjectExists(item->Path))
                    {
                        m_RemovingIndex = row_n;
                        m_OpenErrorPopup = true;
                    }
                    else
                    {
                        ProjectSpace::OpenProject(item->Path);
                        ImGui::CloseCurrentPopup();
                        m_Open = false;
                    }
                }

                //if (ImGui::TableSetColumnIndex(1)) {}
                    //ImGui::TextUnformatted(item->Path);

                ImGui::PopID();
            }
            //ImGui::PopButtonRepeat();
            ImGui::EndTable();

            if (m_OpenErrorPopup) ImGui::OpenPopup("Project Not Found");

            ImGui::SetWindowSize(ImVec2(0.0f, 0.0f));
            if (ImGui::BeginPopupModal("Project Not Found", &m_OpenErrorPopup, ImGuiWindowFlags_AlwaysAutoResize))
            {
                OnProjectNotFoundGui();
                ImGui::EndPopup();
            }

            if (m_OpenFileDialogPopup) ImGui::OpenPopup("File Browser");

            ImGui::SetWindowSize(ImVec2(0.0f, 0.0f));
            if (ImGui::BeginPopupModal("File Browser", &m_OpenFileDialogPopup, ImGuiWindowFlags_AlwaysAutoResize))
            {
                OnFileDialogPopupGui();
                ImGui::EndPopup();
            }

            if (m_OpenNewProjectPopup) ImGui::OpenPopup("New Project");

            ImGui::SetWindowSize(ImVec2(0.0f, 0.0f));
            if (ImGui::BeginPopupModal("New Project", &m_OpenNewProjectPopup, ImGuiWindowFlags_AlwaysAutoResize))
            {
                OnNewProjectPopupGui();
                ImGui::EndPopup();
            }
        }
    }

    GLORY_EDITOR_API void ProjectPopup::OnProjectNotFoundGui()
    {
        ImGui::Text("The project could not be found, would you like to remove it from the list?");
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.x /= 2.0f;
        size.x -= 4.0f;
        size.y = ImGui::GetTextLineHeightWithSpacing();

        if (ImGui::Button("Yes", size))
        {
            m_Projects.erase(m_Projects.begin() + m_RemovingIndex);
            ImGui::CloseCurrentPopup();
            m_OpenErrorPopup = false;
            Save();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", size))
        {
            ImGui::CloseCurrentPopup();
            m_OpenErrorPopup = false;
        }
    }

    GLORY_EDITOR_API void ProjectPopup::OnFileDialogPopupGui()
    {
        static ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_SizingFixedFit;

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("", m_PathText, 100))
        {
            std::filesystem::path path(m_PathText);
            if (std::filesystem::exists(path))
            {
                m_BrowsingPath = path.string();
            }
        }

        std::filesystem::path browsingPath = m_BrowsingPath;

        for (auto it = browsingPath.begin(); it != browsingPath.end(); it++)
        {
            auto subPath = *it;
            std::string name = subPath.string();
            if (name == "" || name == "\\" || name == "/") continue;
            if (ImGui::Button(name.data()))
            {
                std::string pathString = m_BrowsingPath;
                int index = pathString.find(name) + name.size();
                pathString = pathString.substr(0, index);
                if (name == "C:") m_BrowsingPath = "C:/";
                else m_BrowsingPath = pathString;

                strcpy(m_PathText, m_BrowsingPath.data());

                return;
            }
            ImGui::SameLine();
        }

        ImGui::NewLine();

        if (ImGui::BeginTable("File Browser", 1, flags, ImVec2(800.0f, 400.0f), 0.0f))
        {
            ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 0.0f, 0);
            //ImGui::TableSetupColumn("Project Path", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, 1);

            ImGui::TableHeadersRow();
            ImGui::PushButtonRepeat(true);

            int index = 0;
            for (auto& entry : std::filesystem::directory_iterator(m_BrowsingPath))
            {
                if (!entry.is_directory())
                {
                    std::filesystem::path ext = entry.path().extension();
                    if (ext.compare(".gproj")) continue;
                }

                ImGui::PushID(index);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);

                ImGui::TableSetColumnIndex(0);
                ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

                std::filesystem::path path = entry.path();
                std::string pathString = path.string();

                if (ImGui::Selectable(pathString.data(), false, selectable_flags, ImVec2(0, 0.0f)))
                {
                    if (!entry.is_directory())
                    {
                        std::string projectPath = entry.path().string();
                        auto it = std::find_if(m_Projects.begin(), m_Projects.end(), [projectPath](const Project& project)
                        {
                            return project.Path == projectPath;
                        });

                        if (it == m_Projects.end())
                        {
                            Project project;
                            project.Name = m_ProjectNameText;
                            project.Path = projectPath;
                            m_Projects.push_back(project);
                            Save();
                        }

                        ImGui::CloseCurrentPopup();
                        m_OpenFileDialogPopup = false;
                    }
                    else
                    {
                        m_BrowsingPath = entry.path().string();
                        strcpy(m_PathText, m_BrowsingPath.data());
                    }
                }

                ImGui::PopID();
                ++index;
            }

            ImGui::PopButtonRepeat();
            ImGui::EndTable();
        }
    }

    GLORY_EDITOR_API void ProjectPopup::OnNewProjectPopupGui()
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
        ImGui::SetNextItemWidth(600.0f);
        ImGui::InputText("##Project Path", m_PathText, 100);
        ImGui::Spacing();

        bool exists = ProjectSpace::ProjectExists(m_PathText, m_ProjectNameText);

        if (exists)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Project already exists!");
        }

        if (ImGui::Button("Create Project", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)) && !exists)
        {
            Project project;
            project.Name = m_ProjectNameText;
            project.Path = ProjectSpace::NewProject(m_PathText, m_ProjectNameText);
            m_Projects.push_back(project);
            Save();

            ImGui::CloseCurrentPopup();
            m_OpenNewProjectPopup = false;
            m_Open = false;
        }

        if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
            m_OpenNewProjectPopup = false;
        }
    }
}
