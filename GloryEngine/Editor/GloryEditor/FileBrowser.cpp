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
#include "Package.h"

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

    char CreateNameBuffer[1000] = "\0";

    bool StartCreating = false;
    bool IsCreating = false;

    struct CreatingItem
    {
        std::string m_Name;
        std::string m_Icon;
        std::function<void(std::filesystem::path&)> m_Callback;
    } CreatingItem{};

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

    void FileBrowser::BeginCreate(const std::string& name, const std::string& icon, std::function<void(std::filesystem::path&)> callback)
    {
        if (IsCreating) return;
        CreatingItem.m_Name = name;
        CreatingItem.m_Icon = icon;
        CreatingItem.m_Callback = callback;
        StartCreating = true;
        IsCreating = true;

        memcpy(CreateNameBuffer, name.data(), name.length());
        CreateNameBuffer[name.length()] = '\0';
    }

    void FileBrowser::CancelCreate()
    {
        StartCreating = false;
        IsCreating = false;
        CreateNameBuffer[0] = '\0';
    }

    void FileBrowser::OnAsyncImport()
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

        /* We can't import if we're packaging */
        if (IsPackagingBusy()) return;

        /* Is this file already part of the project? */
        ProjectSpace* pProject = ProjectSpace::GetOpenProject();
        if (!pProject) return;
        std::filesystem::path rootPath = pProject->RootPath();
        rootPath.append("Assets");
        if (path.string()._Starts_with(rootPath.string()))
        {
            EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning("File already in project");
            return;
        }

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

        /* Draw preview of creating item */
        DrawCreatingItem();

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

    void FileBrowser::DrawCreatingItem()
    {
        if (!IsCreating) return;

        ImGui::PushID("newItem");
        const ImVec4 buttonColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        const ImVec4 buttonInactiveColor = { buttonColor.x, buttonColor.y, buttonColor.z, 0.0f };

        const float padding = 10.0f;
        const float textHeight = ImGui::CalcTextSize("LABEL").y;
        const ImVec2 itemSize = { m_IconSize + padding * 2.0F, m_IconSize + padding * 3.0f + textHeight };

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::BeginChild("##file", itemSize, false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar(2);

        const ImVec2 cursorPos = ImGui::GetCursorPos();
        EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();
        Texture* pTexture = EditorAssets::GetTexture(CreatingItem.m_Icon.empty() ? "file" : CreatingItem.m_Icon);
        const UUID selectedID = Selection::GetActiveObject() ? Selection::GetActiveObject()->GetUUID() : 0;

        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::Button("##fileItem", itemSize);

        ImGui::PopStyleColor();
        ImGui::SetItemAllowOverlap();
        ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
        ImGui::Image(pTexture ? pRenderImpl->GetTextureID(pTexture) : NULL, ImVec2((float)m_IconSize, (float)m_IconSize));
        DrawCreatingItemName(padding);
        ImGui::EndChild();
        ImGui::PopID();
    }

    void FileBrowser::DrawCreatingItemName(float padding)
    {
        const ImVec2 cursorPos = ImGui::GetCursorPos();

        const float availableWidth = ImGui::GetContentRegionAvail().x - padding;
        const float textWidth = ImGui::CalcTextSize(CreatingItem.m_Name.data()).x;
        const int wraps = (int)(textWidth / (availableWidth - 8.0f));

        ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y - wraps * padding / 1.2f });
        ImGui::SetNextItemWidth(availableWidth - padding);
        ImGui::InputText("##ItemRenaming", CreateNameBuffer, 1000);
        if (StartCreating)
        {
            ImGui::SetKeyboardFocusHere(-1);
        }
        else if (!ImGui::IsItemActive())
        {
            IsCreating = false;
            std::string name = CreateNameBuffer;
            CreateNameBuffer[0] = '\0';
            std::filesystem::path path = FileBrowserItem::GetCurrentPath();
            path.append(name);
            CreatingItem.m_Callback(path);
        }
        StartCreating = false;
    }
}
