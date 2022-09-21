#include <stack>
#include <imgui.h>
#include <AssetDatabase.h>
#include <AssetManager.h>
#include "ProjectSpace.h"
#include "EditorApplication.h"
#include "ContentBrowserItem.h"
#include "EditorAssets.h"
#include "Tumbnail.h"
#include "TumbnailGenerator.h"
#include "Selection.h"
#include "ObjectMenu.h"

namespace Glory::Editor
{
	std::hash<std::string> ContentBrowserItem::m_PathHasher;
	ContentBrowserItem* ContentBrowserItem::m_pSelectedFolder = nullptr;
	std::vector<ContentBrowserItem*> ContentBrowserItem::m_pHistory;
	size_t ContentBrowserItem::m_HistoryIndex = 1;
	std::string ContentBrowserItem::m_HighlightedPath;

	ContentBrowserItem::ContentBrowserItem()
		: m_Name(""), m_pParent(nullptr), m_IsFolder(false), m_SetOpen(false), m_NameBuffer(""), m_EditingName(false), m_StartEditingName(false), m_pChildren(std::vector<ContentBrowserItem*>()), m_Editable(true)
	{
	}

	ContentBrowserItem::ContentBrowserItem(const std::string& name, bool isFolder, ContentBrowserItem* pParent, bool isEditable, const std::string& directoryFilter, std::function<std::filesystem::path()> rootPathFunc)
		: m_Name(name), m_pParent(pParent), m_IsFolder(isFolder), m_SetOpen(false), m_NameBuffer(""), m_EditingName(false), m_StartEditingName(false),
		m_pChildren(std::vector<ContentBrowserItem*>()), m_Editable(isEditable), m_RootPathFunc(rootPathFunc), m_DirectoryFilter(directoryFilter)
	{}

	ContentBrowserItem::~ContentBrowserItem()
	{
		for (size_t i = 0; i < m_pChildren.size(); i++)
		{
			delete m_pChildren[i];
		}
		m_pChildren.clear();
		m_pParent = nullptr;
	}

	ContentBrowserItem* ContentBrowserItem::GetSelectedFolder()
	{
		return m_pSelectedFolder;
	}

	void ContentBrowserItem::SetSelectedFolder(ContentBrowserItem* pItem)
	{
		m_pSelectedFolder = pItem;
	}

	bool ContentBrowserItem::HasParent()
	{
		return m_pParent != nullptr;
	}

	void ContentBrowserItem::Up()
	{
		if (m_pParent == nullptr) return;
		m_pSelectedFolder = m_pParent;
		EraseExcessHistory();
		m_pHistory.push_back(m_pParent);
	}

	void ContentBrowserItem::HistoryUp()
	{
		if (m_HistoryIndex >= m_pHistory.size() || m_pHistory.size() <= 1) return;
		++m_HistoryIndex;
		size_t index = m_pHistory.size() - m_HistoryIndex;
		m_pSelectedFolder = m_pHistory[index];
	}

	void ContentBrowserItem::HistoryDown()
	{
		if (m_HistoryIndex <= 1 || m_pHistory.size() <= 1) return;
		--m_HistoryIndex;
		size_t index = m_pHistory.size() - m_HistoryIndex;
		m_pSelectedFolder = m_pHistory[index];
	}

	void ContentBrowserItem::Change(const std::string& name, bool isFolder)
	{
		if (m_Name == name && m_IsFolder == isFolder) return;

		m_Name = name;
		m_IsFolder = isFolder;
		Refresh();
	}

	void ContentBrowserItem::Refresh()
	{
		if (ProjectSpace::GetOpenProject() == nullptr) return;

		if (m_pParent == nullptr && m_pHistory.size() <= 0) m_pHistory.push_back(this);

		m_CachedPath = BuildPath();
		if (!m_IsFolder) return;

		size_t index = 0;
		for (const auto& entry : std::filesystem::directory_iterator(m_CachedPath))
		{
			size_t actualIndex = index;
			bool directory = entry.is_directory();
			std::filesystem::path path = entry.path();
			std::string pathString = path.string();

			if (!directory)
			{
				// Ignore meta files
				auto ext = path.extension();
				std::filesystem::path metaExtension = std::filesystem::path(".gmeta");
				if (ext.compare(metaExtension) == 0) continue;
				// Ignore files that fail the directory mask
				if (m_DirectoryFilter != "" && pathString.find(m_DirectoryFilter) == std::string::npos) continue;
			}

			bool ignored = false;
			for (size_t i = 0; i < m_IgnoreDirectories.size(); i++)
			{
				const std::string& ignoreDirectory = m_IgnoreDirectories[i];
				if (pathString.find(ignoreDirectory) == std::string::npos) continue;
				ignored = true;
				break;
			}

			if (ignored) continue;

			++index;
			std::filesystem::path::iterator lastDirIT = path.end();
			--lastDirIT;
			std::filesystem::path lastDir = *lastDirIT;
			if (actualIndex >= m_pChildren.size())
			{
				size_t childIndex = m_pChildren.size();
				m_pChildren.push_back(new ContentBrowserItem(lastDir.string(), directory, this, m_Editable, m_DirectoryFilter, m_RootPathFunc));
				m_pChildren[childIndex]->AddIgnoreDirectories(m_IgnoreDirectories);
				ContentBrowserItem* pNewChild = m_pChildren[childIndex];
				pNewChild->Refresh();
				pNewChild->SortChildren();
				continue;
			}

			m_pChildren[actualIndex]->Change(lastDir.string(), directory);
			m_pChildren[actualIndex]->Refresh();
			m_pChildren[actualIndex]->SortChildren();
		}

		std::vector<ContentBrowserItem*>::iterator it = m_pChildren.begin() + index;
		if (it >= m_pChildren.end()) return;
		std::for_each(it, m_pChildren.end(), [](ContentBrowserItem* pChild) { delete pChild; });
		m_pChildren.erase(it, m_pChildren.end());
	}

	void ContentBrowserItem::RefreshSelected(ContentBrowserItem* pRoot)
	{
		if (m_pSelectedFolder != nullptr && m_pSelectedFolder->IsValid()) return;
		m_pSelectedFolder = this;
		m_pHistory.clear();
		m_pHistory.push_back(pRoot);
		m_HistoryIndex = 1;
	}

	std::filesystem::path ContentBrowserItem::BuildPath()
	{
		std::filesystem::path finalPath = "";
		if (m_pParent == nullptr)
			finalPath = m_RootPathFunc();
		else
			finalPath = m_pParent->m_CachedPath;

		finalPath.append(m_Name);
		return finalPath;
	}

	void ContentBrowserItem::DrawDirectoryBrowser()
	{
		if (!m_IsFolder) return;

		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (m_pSelectedFolder == this)
		{
			node_flags |= ImGuiTreeNodeFlags_Selected;
		}

		size_t hash = m_PathHasher(m_CachedPath.string());
		if (m_SetOpen)
		{
			ImGui::SetNextItemOpen(true);
			m_SetOpen = false;
		}

		bool node_open = ImGui::TreeNodeEx((void*)hash, node_flags, m_Name.data());
		if (ImGui::IsItemClicked())
		{
			m_pSelectedFolder = this;
			EraseExcessHistory();
			m_pHistory.push_back(this);
		}

		if (node_open)
		{
			for (size_t i = 0; i < m_pChildren.size(); i++)
			{
				m_pChildren[i]->DrawDirectoryBrowser();
			}
			ImGui::TreePop();
		}
	}

	void ContentBrowserItem::DrawFileBrowser(int iconSize)
	{
		if (!m_pSelectedFolder) return;

		ImVec2 windowSize = ImGui::GetWindowSize();

		float width = windowSize.x;

		int columns = (int)(width / (iconSize + 22.0f)) - 1;
		if (columns <= 0) columns = 1;

		//ImGui::ImageButton(NULL, ImVec2(64.0f, 64.0f));

		ImGui::Columns(columns, NULL, false);

		for (size_t i = 0; i < m_pSelectedFolder->m_pChildren.size(); i++)
		{
			ContentBrowserItem* pChild = m_pSelectedFolder->m_pChildren[i];

			int columnIndex = (i % columns) - 1;
			ImGui::SetColumnWidth(columnIndex, (float)iconSize + 22.0f);
			pChild->DrawFileItem(iconSize);
			int mod = i % columns;
			if (mod != 0) ImGui::SameLine();
			ImGui::NextColumn();
		}
	}

	void ContentBrowserItem::DrawCurrentPath()
	{
		if (!m_pSelectedFolder) return;

		std::vector<ContentBrowserItem*> pPathTrace;
		pPathTrace.push_back(m_pSelectedFolder);
		ContentBrowserItem* pParent = m_pSelectedFolder->m_pParent;
		while (pParent != nullptr)
		{
			pPathTrace.push_back(pParent);
			pParent = pParent->m_pParent;
		}

		std::reverse(pPathTrace.begin(), pPathTrace.end());
		for (size_t i = 0; i < pPathTrace.size(); i++)
		{
			ContentBrowserItem* pItem = pPathTrace[i];
			size_t hash = m_PathHasher(pItem->m_CachedPath.string());
			ImGui::PushID(hash);
			if (ImGui::Button(pItem->m_Name.c_str()))
			{
				m_pSelectedFolder = pItem;
				EraseExcessHistory();
				m_pHistory.push_back(pItem);
			}
			ImGui::PopID();
			ImGui::SameLine();
		}
	}

	void ContentBrowserItem::DrawFileItem(int iconSize)
	{
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();
		if (m_IsFolder)
		{
			Texture* pFolderTexture = EditorAssets::GetTexture("folder");

			ImGui::ImageButton(pRenderImpl->GetTextureID(pFolderTexture), ImVec2((float)iconSize, (float)iconSize));
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				m_pSelectedFolder = this;
				EraseExcessHistory();
				m_pHistory.push_back(this);
				SetOpen();
			}

			if (ImGui::IsItemClicked(1))
			{
				m_HighlightedPath = m_CachedPath.string();
				ObjectMenu::Open(nullptr, m_Editable ? ObjectMenuType::T_Folder : ObjectMenuType::T_ModuleFolder);
			}

			DrawName();
			return;
		}

		std::filesystem::path assetPath = ProjectSpace::GetOpenProject()->RootPath();
		assetPath.append("Assets");
		std::filesystem::path relativePath = m_CachedPath.lexically_relative(assetPath);
		if (relativePath == "") relativePath = m_CachedPath;
		UUID uuid = AssetDatabase::GetAssetUUID(relativePath.string());
		Texture* pTexture = Tumbnail::GetTumbnail(uuid);

		ImGui::ImageButton(pTexture ? pRenderImpl->GetTextureID(pTexture) : NULL, ImVec2((float)iconSize, (float)iconSize));

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			ResourceMeta meta;
			AssetDatabase::GetResourceMeta(uuid, meta);
			BaseTumbnailGenerator* pGenerator = Tumbnail::GetGenerator(meta.Hash());
			if (!pGenerator) return;
			pGenerator->OnFileDoubleClick(uuid);
		}

		if (ImGui::IsItemClicked(0))
		{
			Resource* pAsset = AssetManager::GetAssetImmediate(uuid);
			Selection::SetActiveObject(pAsset);
		}

		if (ImGui::IsItemClicked(1))
		{
			m_HighlightedPath = m_CachedPath.string();
			Resource* pAsset = AssetManager::GetAssetImmediate(uuid);
			ObjectMenu::Open(pAsset, m_Editable ? ObjectMenuType::T_Resource : ObjectMenuType::T_ModuleResource);
		}

		DrawName();
	}

	bool ContentBrowserItem::IsValid()
	{
		return std::filesystem::exists(m_CachedPath);
	}

	void ContentBrowserItem::SetOpen()
	{
		if (m_pParent != nullptr) m_pParent->SetOpen();
		m_SetOpen = true;
	}

	void ContentBrowserItem::SortChildren()
	{
		std::sort(m_pChildren.begin(), m_pChildren.end(), [&](ContentBrowserItem* pA, ContentBrowserItem* pB)
		{
			if (pA->m_IsFolder && !pB->m_IsFolder) return true;
			if (!pA->m_IsFolder && pB->m_IsFolder) return false;
			return pA->m_Name < pB->m_Name;
		});
	}

	std::filesystem::path ContentBrowserItem::GetCurrentPath()
	{
		return m_pSelectedFolder != nullptr ? m_pSelectedFolder->BuildPath() : ProjectSpace::GetOpenProject()->RootPath();
	}

	ContentBrowserItem* ContentBrowserItem::GetChildByName(const std::string& name, bool folder)
	{
		auto it = std::find_if(m_pChildren.begin(), m_pChildren.end(), [&](ContentBrowserItem* pChild) {return pChild->m_Name.find(name) != std::string::npos && pChild->m_IsFolder == folder; });
		if (it == m_pChildren.end()) return nullptr;
		return *it;
	}

	void ContentBrowserItem::BeginRename()
	{
		m_StartEditingName = true;
		m_EditingName = true;
		std::string name = m_CachedPath.filename().replace_extension().string();
		memcpy(m_NameBuffer, name.data(), name.length());
		m_NameBuffer[name.length()] = '\0';
	}

	bool ContentBrowserItem::IsEditable() const
	{
		return m_Editable;
	}

	const std::string& ContentBrowserItem::GetHighlightedPath()
	{
		return m_HighlightedPath;
	}

	void ContentBrowserItem::AddIgnoreDirectory(const std::string& directory)
	{
		m_IgnoreDirectories.push_back(directory);
	}

	void ContentBrowserItem::AddIgnoreDirectories(const std::vector<std::string>& directories)
	{
		m_IgnoreDirectories = directories;
	}

	void ContentBrowserItem::EraseExcessHistory()
	{
		if (m_HistoryIndex <= 1) return;
		std::vector<ContentBrowserItem*>::iterator it = m_pHistory.end() - (m_HistoryIndex - 1);
		if (it >= m_pHistory.end()) return;
		m_pHistory.erase(it, m_pHistory.end());
		m_HistoryIndex = 1;
	}

	void ContentBrowserItem::DrawName()
	{
		if (m_EditingName || m_StartEditingName)
		{
			ImGui::InputText("##ItemRenaming", m_NameBuffer, 1000);
			if (m_StartEditingName)
			{
				ImGui::SetKeyboardFocusHere(-1);
			}
			if (!m_StartEditingName && !ImGui::IsItemActive())
			{
				m_EditingName = false;
				std::filesystem::path extension = m_CachedPath.extension();
				std::string newName = std::filesystem::path(m_NameBuffer).replace_extension(extension).string();
				if (newName == "" || newName == m_Name) return;
				m_Name = newName;
				m_NameBuffer[0] = '\0';

				std::filesystem::path newPath = BuildPath();
				if (m_CachedPath == newPath) return;

				std::filesystem::rename(m_CachedPath, newPath);
				if(!m_IsFolder) std::filesystem::rename(m_CachedPath.string() + ".gmeta", newPath.string() + ".gmeta");

				std::filesystem::path assetPath = Game::GetAssetPath();
				AssetDatabase::UpdateAssetPaths(m_CachedPath.lexically_relative(assetPath).string(), newPath.lexically_relative(assetPath).string());
				AssetDatabase::Save();
				m_CachedPath = newPath;
				Refresh();
			}
			m_StartEditingName = false;
		}
		else ImGui::Text(m_CachedPath.filename().replace_extension().string().c_str());
	}

	std::filesystem::path ContentBrowserItem::DefaultRootPathFunc()
	{
		std::string projectRootPath = "";
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		if (pProject != nullptr) projectRootPath = pProject->RootPath();
		return projectRootPath;
	}
}
