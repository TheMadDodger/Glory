#include "FileBrowser.h"
#include "Selection.h"
#include "Tumbnail.h"
#include "EditorAssets.h"
#include "EditorApplication.h"
#include "EditorRenderImpl.h"
#include "ObjectMenu.h"
#include "ImGuiHelpers.h"

#include <imgui.h>
#include <AssetCallbacks.h>

namespace Glory::Editor
{
    int FileBrowser::m_IconSize = 128;

	FileBrowser::FileBrowser() : EditorWindowTemplate("File Browser", 1600.0f, 600.0f),
        m_I(0), m_SearchBuffer("\0"), m_pRootItems(std::vector<FileBrowserItem*>())
	{
		m_Resizeable = true;
        m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar;

        AssetCallbacks::RegisterCallback(CallbackType::CT_AssetRegistered, [&](UUID uuid, const ResourceMeta& meta, Resource* pResource)
        {
            //RefreshContentBrowser();
        });
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
    }

    void FileBrowser::OnClose()
    {
        for (size_t i = 0; i < m_pRootItems.size(); i++)
        {
            delete m_pRootItems[i];
        }
        m_pRootItems.clear();
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
        ImVec2 size(14.0f, 14.0f);

        Texture* pBackTexture = EditorAssets::GetTexture("back");
        Texture* pForwardTexture = EditorAssets::GetTexture("forward");
        Texture* pUpTexture = EditorAssets::GetTexture("up");
        Texture* pRefreshTexture = EditorAssets::GetTexture("refresh");

        EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();

        if (ImGui::ImageButton(pRenderImpl->GetTextureID(pBackTexture), size, ImVec2(0,0), ImVec2(1,1), -1, ImVec4(0, 0, 0, 0), ImVec4(0.5, 0.5, 0.5, 1)))
        {
            FileBrowserItem::HistoryUp();
        }

        ImGui::SameLine();
        if (ImGui::ImageButton(pRenderImpl->GetTextureID(pForwardTexture), size))
        {
            FileBrowserItem::HistoryDown();
        }

        ImGui::SameLine();
        if (ImGui::ImageButton(pRenderImpl->GetTextureID(pUpTexture), size))
        {
            FileBrowserItem::GetSelectedFolder()->Up();
        }

        ImGui::SameLine();
        if (ImGui::ImageButton(pRenderImpl->GetTextureID(pRefreshTexture), size))
        {
            RefreshContentBrowser();
        }
    }

    void FileBrowser::DrawSearchBar()
    {
        float regionWidth = ImGui::GetWindowContentRegionMax().x;
        float width = 300.0f;
        float padding = 10.0f;

        const char* searchText = "Search: ";
        float textWidth = ImGui::CalcTextSize(searchText).x;

        ImGui::SameLine(regionWidth - width - textWidth - padding);
        ImGui::Text(searchText);
        ImGui::SameLine(regionWidth - width - padding);
        ImGui::SetNextItemWidth(width);
        ImGui::InputText("##Search", m_SearchBuffer, 100);
    }

    void FileBrowser::DrawFileBrowser()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::BeginChild("FileBrowser", ImVec2(0, 0), true, window_flags);
        ImVec2 min = ImGui::GetWindowContentRegionMin();
        ImVec2 max = ImGui::GetWindowContentRegionMax();
        ImVec2 pos = ImGui::GetWindowPos();

        FileBrowserItem* pSelected = FileBrowserItem::GetSelectedFolder();
        if (pSelected->IsEditable() && ImGui::IsMouseHoveringRect(pos + min, pos + max) && ImGui::IsMouseClicked(1)) ObjectMenu::Open(nullptr, ObjectMenuType::T_ContentBrowser);
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
    }


    void FileBrowser::LoadItems()
    {
        for (size_t i = 0; i < m_pRootItems.size(); i++)
        {
            m_pRootItems[i]->Refresh();
            m_pRootItems[i]->SortChildren();
        }
    }
}
