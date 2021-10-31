#include <imgui.h>
#include "ContentBrowser.h"
#include "Selection.h"
#include "Tumbnail.h"
#include "EditorAssets.h"
//#include <MetaData.h>
//#include "AssetManager.h"
#include "EditorApplication.h"
#include "EditorRenderImpl.h"

#include <GLTexture.h>
#include <ProjectSpace.h>

namespace Glory::Editor
{
    int ContentBrowser::m_IconSize = 128;

	ContentBrowser::ContentBrowser() : EditorWindowTemplate("Content Browser", 1600.0f, 600.0f),
        m_I(0), m_SearchBuffer("\0"), m_Refresh(false), m_pRootItem(nullptr)
	{
		m_Resizeable = true;
        m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar;
	}

	ContentBrowser::~ContentBrowser()
	{
        delete m_pRootItem;
        m_pRootItem = nullptr;
	}

	void ContentBrowser::OnGUI()
	{
        RefreshContentBrowser();

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
                    FileBrowser();
                    //ImGui::TableNextRow(ImGuiTableRowFlags_None, rows_height);
                    //ImGui::SliderInt("Size", &m_IconSize, 32.0f, 256.0f);
                    ImGui::EndTable();
                }
            }
            ImGui::EndTable();
        }
	}

    void ContentBrowser::DirectoryBrowser()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::BeginChild("DirectoryBrowser", ImVec2(0, 0), false, window_flags);
        if (m_pRootItem) m_pRootItem->DrawDirectoryBrowser();
        ImGui::EndChild();
    }

    void ContentBrowser::FileBrowserMenu()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
        ImGui::BeginChild("DirectoryBrowser", ImVec2(0, 0), false, window_flags);
        DrawPathControls();
        ImGui::SameLine();
        ContentBrowserItem::DrawCurrentPath();
        DrawSearchBar();
        ImGui::EndChild();
    }

    void ContentBrowser::DrawPathControls()
    {
        ImVec2 size(14.0f, 14.0f);

        Texture* pBackTexture = EditorAssets::GetTexture("back");
        Texture* pForwardTexture = EditorAssets::GetTexture("forward");
        Texture* pUpTexture = EditorAssets::GetTexture("up");
        Texture* pRefreshTexture = EditorAssets::GetTexture("refresh");

        EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();

        if (ImGui::ImageButton(pRenderImpl->GetTextureID(pBackTexture), size, ImVec2(0,0), ImVec2(1,1), -1, ImVec4(0, 0, 0, 0), ImVec4(0.5, 0.5, 0.5, 1)))
        {
            ContentBrowserItem::HistoryUp();
        }

        ImGui::SameLine();
        if (ImGui::ImageButton(pRenderImpl->GetTextureID(pForwardTexture), size))
        {
            ContentBrowserItem::HistoryDown();
        }

        ImGui::SameLine();
        if (ImGui::ImageButton(pRenderImpl->GetTextureID(pUpTexture), size))
        {
            ContentBrowserItem::GetSelectedFolder()->Up();
        }

        ImGui::SameLine();
        if (ImGui::ImageButton(pRenderImpl->GetTextureID(pRefreshTexture), size))
        {
            RefreshContentBrowser();
        }
    }

    void ContentBrowser::DrawSearchBar()
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
        ImGui::InputText("", m_SearchBuffer, 100);
    }

    void ContentBrowser::FileBrowser()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::BeginChild("FileBrowser", ImVec2(0, 0), true, window_flags);
        ContentBrowserItem::DrawFileBrowser(m_IconSize);
        ImGui::EndChild();
    }

    void ContentBrowser::RefreshContentBrowser()
    {
        if (ProjectSpace::GetOpenProject() == nullptr) return;

        if (m_Refresh) return;
        if (m_pRootItem == nullptr) m_pRootItem = new ContentBrowserItem("Assets", true, nullptr);
        m_pRootItem->Refresh();
        m_pRootItem->SortChildren();
        m_pRootItem->RefreshSelected(m_pRootItem);
        m_Refresh = true;
    }
}
