#include "stdafx.h"
#include "ContentBrowser.h"
#include "Selection.h"
#include "Tumbnail.h"
//#include <MetaData.h>
//#include "AssetManager.h"

namespace Glory::Editor
{
    int ContentBrowser::m_IconSize = 64;

	ContentBrowser::ContentBrowser() : EditorWindowTemplate("Content Browser", 1600.0f, 600.0f)
	{
		m_Resizeable = true;
        //m_RootAssetPath = BaseGame::GetAssetRootPath();
        //m_SelectedPath = BaseGame::GetAssetRootPath();
	}

	ContentBrowser::~ContentBrowser()
	{
	}

	void ContentBrowser::OnGUI()
	{
        // NB: Future columns API should allow automatic horizontal borders.
        //ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
        //DirectoryBrowser();
        //ImGui::SameLine();
        //FileBrowser();
	}

    void ContentBrowser::ProcessDirectory(std::filesystem::path path)
    {
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        bool hasSubFolders = false;
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (!entry.is_directory()) continue;
            node_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
            hasSubFolders = true;
            break;
        }

        if (!m_SelectedPath.compare(path))
        {
            node_flags |= ImGuiTreeNodeFlags_Selected;
        }

        auto it = path.end();
        --it;
        std::filesystem::path folderPath = *it;

        auto name = folderPath.string();
        size_t hash = m_Hasher(path.string());
        bool node_open = ImGui::TreeNodeEx((void*)hash, node_flags, name.c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectedPath = path;
        }

        if (node_open && hasSubFolders)
        {
            for (const auto& entry : std::filesystem::directory_iterator(path))
            {
                if (entry.is_directory())
                {
                    ProcessDirectory(entry.path());
                    continue;
                }
            }
            ImGui::TreePop();
        }
    }

    void ContentBrowser::ProcessFile(std::filesystem::path path)
    {
        // Leaf: The only reason we have a TreeNode at all is to allow selection of the leaf. Otherwise we can use BulletText() or TreeAdvanceToLabelPos()+Text().
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
        size_t hash = m_Hasher(path.string());
        ImGui::TreeNodeEx((void*)hash, node_flags, path.filename().string().c_str());
        if (ImGui::IsItemClicked())
        {
            //Selection::SetActiveObject(pChild);
        }
        if (ImGui::IsItemClicked(1))
        {
            //m_CurrentRightClickedObject = pChild;
            ImGui::OpenPopup("object_menu_popup" + m_I);
        }
        if (ImGui::BeginPopup("object_menu_popup" + m_I))
        {
            //ObjectMenu();
            ImGui::EndPopup();
        }
    }

    void ContentBrowser::DirectoryBrowser()
    {
        auto assetRootPath = m_RootAssetPath;//BaseGame::GetAssetRootPath();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
        ImGui::BeginChild("DirectoryBrowser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, m_WindowDimensions.y), false, window_flags);

        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (!m_SelectedPath.compare(m_RootAssetPath))
        {
            node_flags |= ImGuiTreeNodeFlags_Selected;
        }
        bool node_open = ImGui::TreeNodeEx((void*)("Assets"), node_flags, "Assets");
        if (ImGui::IsItemClicked())
        {
            m_SelectedPath = m_RootAssetPath;
        }

        if (node_open)
        {
            for (const auto& entry : std::filesystem::directory_iterator(assetRootPath))
            {
                if (entry.is_directory())
                {
                    ProcessDirectory(entry.path());
                    continue;
                }
            }
            ImGui::TreePop();
        }

        ImGui::EndChild();
    }

    void ContentBrowser::FileBrowser()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
        //ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild("FileBrowser", ImVec2(0, m_WindowDimensions.y), true, window_flags);

        std::filesystem::path currentPath = m_SelectedPath;
        auto sa = currentPath.relative_path();
        std::for_each(currentPath.begin(), currentPath.end(), [&](std::filesystem::path path)
        {
            if (path.compare(".") && path.compare(".."))
            {
                auto name = path.string();
                if (ImGui::Button(name.c_str()))
                {
                    std::string pathString = currentPath.string();
                    int index = pathString.find(name) + name.size();
                    pathString = pathString.substr(0, index);
                    m_SelectedPath = pathString;
                }
                ImGui::SameLine();
            }
        });

        ImGui::SliderInt("Size", &m_IconSize, 32.0f, 256.0f);

        ImGui::NewLine();


        float windowFactor = 0.7f;
        float width = m_WindowDimensions.x * windowFactor;

        int columns = (int)(width / (m_IconSize + 22.0f)) - 1;
        if (columns <= 0) columns = 1;

        //ImGui::ImageButton(NULL, ImVec2(64.0f, 64.0f));

        ImGui::Columns(columns, NULL, false);
        int index = 1;
        for (const auto& entry : std::filesystem::directory_iterator(m_SelectedPath))
        {
            ImGui::SetColumnWidth((index % columns) - 1, (float)m_IconSize + 22.0f);

            std::filesystem::path path = entry.path();
            if (entry.is_directory())
            {
                ImGui::ImageButton(/*(void*)Tumbnail::GetFolderTumbnail()->GetID()*/ NULL, ImVec2((float)m_IconSize, (float)m_IconSize));
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    m_SelectedPath = path;
                }

                std::filesystem::path::iterator lastDirIT = path.end();
                --lastDirIT;
                std::filesystem::path lastDir = *lastDirIT;
                ImGui::Text(lastDir.string().c_str());

                int mod = index % columns;
                if (mod != 0) ImGui::SameLine();
                ++index;
                ImGui::NextColumn();
                continue;
            }
            
            auto ext = path.extension();
            std::filesystem::path metaExtension = std::filesystem::path(".meta");
            if (ext.compare(metaExtension) != 0)
            {
                NonMetaFile(path);
                continue;
            }

            //Spartan::Serialization::MetaData metaData = Spartan::Serialization::MetaData::Read(path.string());
            //TextureData* pTexture = Tumbnail::GetTumbnail(metaData);

            ImGui::ImageButton(/*pTexture ? (void*)pTexture->GetID() : NULL*/ NULL, ImVec2((float)m_IconSize, (float)m_IconSize));

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
            {
                //Content* pAsset = AssetManager::GetAsset(metaData.m_GUID);
                //Selection::SetActiveObject(pAsset);
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                //OnFileDoubleClick(path);
            }

            ImGui::Text(path.filename().replace_extension().string().c_str());

            int mod = index % columns;
            if (mod != 0) ImGui::SameLine();
            ++index;

            ImGui::NextColumn();
        }

        ImGui::EndChild();
    }

    void ContentBrowser::NonMetaFile(std::filesystem::path& path)
    {
        //TextureData* pTexture = Tumbnail::GetTumbnail(path.extension().string());
        //
        //ImGui::ImageButton(pTexture ? (void*)pTexture->GetID() : NULL, ImVec2((float)m_IconSize, (float)m_IconSize));
        //
        //if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        //{
        //    OnFileDoubleClick(path);
        //}
    }
}