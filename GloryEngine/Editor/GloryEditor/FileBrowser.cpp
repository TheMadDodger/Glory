#include "FileBrowser.h"
#include "Selection.h"
#include "Tumbnail.h"
#include "EditorAssets.h"
#include "EditorApplication.h"
#include "EditorRenderImpl.h"
#include "ObjectMenu.h"
#include "ImGuiHelpers.h"
#include "EditableEntity.h"
#include "EntityEditor.h"

#include <imgui.h>
#include <EditorAssetCallbacks.h>
#include <EditorAssetDatabase.h>
#include <EditorAssetsWatcher.h>
#include <Dispatcher.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
    int FileBrowser::m_IconSize = 128;
    bool FileBrowser::m_SearchInCurrent = false;

    UUID AssetsFileWatchListenerID = 0;

	FileBrowser::FileBrowser() : EditorWindowTemplate("File Browser", 1600.0f, 600.0f),
        m_I(0), m_pRootItems(std::vector<FileBrowserItem*>())
	{
		m_Resizeable = true;
        m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar;
	}

	FileBrowser::~FileBrowser()
	{
        for (size_t i = 0; i < m_pRootItems.size(); i++)
        {
            delete m_pRootItems[i];
        }
        m_pRootItems.clear();
	}

    std::filesystem::path FileBrowser::GetCurrentPath()
    {
        return FileBrowserItem::GetCurrentPath();
    }

	void FileBrowser::OnGUI()
	{
        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing() + 4.0f;
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable;// | ImGuiTableFlags_BordersOuter;// | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

        if (ImGui::BeginTable("ContentBrowserTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextColumn();
            DirectoryBrowser();
            ImGui::TableNextColumn();
            {
                float rows_height = TEXT_BASE_HEIGHT;
                float fileBrowserHeight = ImGui::GetWindowContentRegionMax().y - rows_height;
                if (ImGui::BeginTable("FileBrowserTable", 1, ImGuiTableFlags_None))
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, rows_height);
                    FileBrowserMenu();
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, fileBrowserHeight);
                    DrawFileBrowser();
                    //ImGui::TableNextRow(ImGuiTableRowFlags_None, rows_height);
                    //ImGui::SliderInt("Size", &m_IconSize, 32.0f, 256.0f);
                    ImGui::EndTable();
                }
            }
            ImGui::EndTable();
        }
	}

    void FileBrowser::OnOpen()
    {
        EditorAssetDatabase::RegisterAsyncImportCallback(OnAsyncImport);

        m_pRootItems.push_back(new FileBrowserItem("Assets", true, nullptr, true));
        m_pRootItems.push_back(new FileBrowserItem("Modules", true, nullptr, false, "\\Assets\\", []() { return "./"; }));
        m_pRootItems[1]->AddIgnoreDirectory("\\Editor");
        m_pRootItems[1]->AddIgnoreDirectory("\\Resources");
        m_pRootItems[1]->AddIgnoreDirectory("\\Dependencies");
        LoadItems();

        for (size_t i = 0; i < m_pRootItems.size(); i++)
        {
            m_pRootItems[i]->RefreshSelected(m_pRootItems[i]);
        }

        AssetsFileWatchListenerID = EditorAssetsWatcher::AssetsFileWatchEvents().AddListener([&](const AssetsFileWatchEvent& e) {
            if (e.Action == efsw::Action::Modified) return;
            FileBrowserItem::GetSelectedFolder()->Refresh();
            FileBrowserItem::GetSelectedFolder()->SortChildren();
            RefreshSearch();
        });
    }

    void FileBrowser::OnClose()
    {
        EditorAssetDatabase::RegisterAsyncImportCallback(NULL);

        for (size_t i = 0; i < m_pRootItems.size(); i++)
        {
            delete m_pRootItems[i];
        }
        m_pRootItems.clear();

        EditorAssetsWatcher::AssetsFileWatchEvents().RemoveListener(AssetsFileWatchListenerID);
    }

    void FileBrowser::BeginRename(const std::string& name, bool folder)
    {
        FileBrowserItem* pCurrentFolder = FileBrowserItem::GetSelectedFolder();
        FileBrowserItem* pChildToRename = pCurrentFolder->GetChildByName(name, folder);
        if (pChildToRename == nullptr) return;
        if (!pCurrentFolder->IsEditable() || !pChildToRename->IsEditable()) return;
        pChildToRename->BeginRename();
    }

    void FileBrowser::LoadProject()
    {
        FileBrowser* pWindow = GetWindow<FileBrowser>();
        FileBrowserItem::SetSelectedFolder(pWindow->m_pRootItems[0]);
        pWindow->LoadItems();
        pWindow->RefreshContentBrowser();
    }

    void FileBrowser::OnAsyncImport(Resource*)
    {
        FileBrowser* pWindow = GetWindow<FileBrowser>();
        std::filesystem::path currentPath = FileBrowserItem::GetCurrentPath();
        FileBrowserItem::SetSelectedFolder(pWindow->m_pRootItems[0]);
        pWindow->LoadItems();
        if (std::filesystem::exists(currentPath))
        {
            pWindow->m_pRootItems[0]->SetSelectedFolder(currentPath);
            if (FileBrowserItem::GetSelectedFolder() == nullptr)
                FileBrowserItem::SetSelectedFolder(pWindow->m_pRootItems[0]);
        }
        pWindow->RefreshContentBrowser();
    }

    void FileBrowser::OnFileDragAndDrop(const std::filesystem::path& path)
    {
        /* Ignore folders */
        if (std::filesystem::is_directory(path)) return;

        /* Is this file already part of the project? */
        ProjectSpace* pProject = ProjectSpace::GetOpenProject();
        if (!pProject) return;
        std::filesystem::path rootPath = pProject->RootPath();
        std::filesystem::path relativePath = std::filesystem::relative(path, rootPath);
        if (relativePath.empty() || relativePath.native()[0] != '.') return;

        /* TODO: Prompt the user with a popup for import settings if relevant */

        /* Copy the file to the current folder */
        std::filesystem::path destination = FileBrowserItem::GetCurrentPath();
        if (destination.string().find("./Modules") != std::string::npos) destination = rootPath.append("Assets");
        destination.append(path.filename().string());
        std::filesystem::copy(path, destination);

        /* Import the asset */
        EditorAssetDatabase::ImportAssetAsync(destination.string());
    }

    void FileBrowser::DirectoryBrowser()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::BeginChild("DirectoryBrowser", ImVec2(0, 0), false, window_flags);
        for (size_t i = 0; i < m_pRootItems.size(); i++)
        {
            m_pRootItems[i]->DrawDirectoryBrowser();
        }
        ImGui::EndChild();
    }

    void FileBrowser::FileBrowserMenu()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
        ImGui::BeginChild("DirectoryBrowser", ImVec2(0, 0), false, window_flags);
        DrawPathControls();
        ImGui::SameLine();
        FileBrowserItem::DrawCurrentPath();
        DrawSearchBar();
        ImGui::EndChild();
    }

    void FileBrowser::DrawPathControls()
    {
        const ImVec2 size(24.0f, 0.0f);

        if (ImGui::Button(ICON_FA_CARET_LEFT, size))
        {
            FileBrowserItem::HistoryUp();
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_CARET_RIGHT, size))
        {
            FileBrowserItem::HistoryDown();
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_CARET_UP, size))
        {
            FileBrowserItem::GetSelectedFolder()->Up();
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROWS_ROTATE, size))
        {
            RefreshContentBrowser();
        }
    }

    void FileBrowser::DrawSearchBar()
    {
        float regionWidth = ImGui::GetWindowContentRegionMax().x;
        float width = 300.0f;
        float padding = 0.0f;

        const char* folderToggleText = m_SearchInCurrent ?
            ICON_FA_FOLDER_CLOSED " " : ICON_FA_FOLDER_OPEN " ";
        float folderToggleTextWidth = ImGui::CalcTextSize(ICON_FA_FOLDER_OPEN " ").x;

        const char* searchText = ICON_FA_MAGNIFYING_GLASS " ";
        float searchTextWidth = ImGui::CalcTextSize(searchText).x;

        bool forceFilter = false;

        ImGui::SameLine(regionWidth - width - searchTextWidth - folderToggleTextWidth - padding);
        ImGui::Text(folderToggleText);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Search %s", m_SearchInCurrent ? "in current folder" : "globally");
            if (ImGui::IsMouseClicked(0))
            {
                m_SearchInCurrent = !m_SearchInCurrent;
                forceFilter = true;
            }
        }

        ImGui::SameLine(regionWidth - width - searchTextWidth - padding);
        ImGui::Text(searchText);
        ImGui::SameLine(regionWidth - width - padding);
        ImGui::SetNextItemWidth(width);
        if (ImGui::InputText("##Search", FileBrowserItem::m_SearchBuffer, 1000) || forceFilter)
        {
            RefreshSearch();
        }
    }

    void FileBrowser::DrawFileBrowser()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::BeginChild("FileBrowser", ImVec2(0, 0), true, window_flags);
        /* TODO: This will need to become a custom DND target with internal ImGui::BeginDragDropTargetCustom */
        DND{ { ResourceTypes::GetHash<EditableEntity>() } }.HandleDragAndDropWindowTarget([&](uint32_t hash, const ImGuiPayload* payload) {
            const ObjectPayload objectPayload = *(const ObjectPayload*)payload->Data;
            EditableEntity* pEntity = GetEditableEntity(objectPayload.EntityID, objectPayload.SceneID);
            FileBrowserItem::ObjectDNDEventDispatcher().Dispatch({ GetCurrentPath(), pEntity });
            return;
        });

        if (FileBrowserItem::m_Dirty)
            RefreshContentBrowser();

        FileBrowserItem* pSelected = FileBrowserItem::GetSelectedFolder();
        if (pSelected->IsEditable() && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1)) ObjectMenu::Open(nullptr, ObjectMenuType::T_ContentBrowser);
        FileBrowserItem::DrawFileBrowser(m_IconSize);
        ImGui::EndChild();
    }

    void FileBrowser::RefreshContentBrowser()
    {
        if (ProjectSpace::GetOpenProject() == nullptr) return;
        FileBrowserItem* pSelected = FileBrowserItem::GetSelectedFolder();
        if (pSelected == nullptr) return;
        pSelected->Refresh();
        pSelected->SortChildren();
        RefreshSearch();
        FileBrowserItem::m_Dirty = false;
    }


    void FileBrowser::LoadItems()
    {
        for (size_t i = 0; i < m_pRootItems.size(); i++)
        {
            m_pRootItems[i]->Refresh();
            m_pRootItems[i]->SortChildren();
        }
    }

    void FileBrowser::RefreshSearch()
    {
        if (m_SearchInCurrent)
        {
            FileBrowserItem::m_pSearchResultCache.clear();
            FileBrowserItem::PerformSearch(FileBrowserItem::m_pSelectedFolder);
        }
        else
        {
            FileBrowserItem::PerformSearch(m_pRootItems);
        }
    }
}
