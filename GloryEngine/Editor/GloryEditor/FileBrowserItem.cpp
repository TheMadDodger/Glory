#include <stack>
#include <imgui.h>
#include <AssetManager.h>
#include "EditorAssetDatabase.h"
#include "ProjectSpace.h"
#include "EditorApplication.h"
#include "FileBrowserItem.h"
#include "EditorAssets.h"
#include "Tumbnail.h"
#include "TumbnailGenerator.h"
#include "Selection.h"
#include "ObjectMenu.h"

namespace Glory::Editor
{
	std::hash<std::string> FileBrowserItem::m_PathHasher;
	FileBrowserItem* FileBrowserItem::m_pSelectedFolder = nullptr;
	std::vector<FileBrowserItem*> FileBrowserItem::m_pHistory;
	size_t FileBrowserItem::m_HistoryIndex = 1;
	std::string FileBrowserItem::m_HighlightedPath;

	FileBrowserItem::FileBrowserItem()
		: m_Name(""), m_pParent(nullptr), m_IsFolder(false), m_SetOpen(false), m_NameBuffer(""), m_EditingName(false), m_StartEditingName(false), m_pChildren(std::vector<FileBrowserItem*>()), m_Editable(true)
	{
	}

	FileBrowserItem::FileBrowserItem(const std::string& name, bool isFolder, FileBrowserItem* pParent, bool isEditable, const std::string& directoryFilter, std::function<std::filesystem::path()> rootPathFunc)
		: m_Name(name), m_pParent(pParent), m_IsFolder(isFolder), m_SetOpen(false), m_NameBuffer(""), m_EditingName(false), m_StartEditingName(false),
		m_pChildren(std::vector<FileBrowserItem*>()), m_Editable(isEditable), m_RootPathFunc(rootPathFunc), m_DirectoryFilter(directoryFilter)
	{}

	FileBrowserItem::~FileBrowserItem()
	{
		for (size_t i = 0; i < m_pChildren.size(); i++)
		{
			delete m_pChildren[i];
		}
		m_pChildren.clear();
		m_pParent = nullptr;
	}

	FileBrowserItem* FileBrowserItem::GetSelectedFolder()
	{
		return m_pSelectedFolder;
	}

	void FileBrowserItem::SetSelectedFolder(const std::filesystem::path& path)
	{
		const std::filesystem::path relative = path.lexically_relative(Game::GetAssetPath());
		FileBrowserItem* pChild = this;
		for (auto subPath : relative)
		{
			pChild = pChild->GetChildByName(subPath.string(), true);
			if (!pChild) break;
		}
		SetSelectedFolder(pChild);
	}

	void FileBrowserItem::SetSelectedFolder(FileBrowserItem* pItem)
	{
		m_pSelectedFolder = pItem;
	}

	bool FileBrowserItem::HasParent()
	{
		return m_pParent != nullptr;
	}

	void FileBrowserItem::Up()
	{
		if (m_pParent == nullptr) return;
		m_pSelectedFolder = m_pParent;
		EraseExcessHistory();
		m_pHistory.push_back(m_pParent);
	}

	void FileBrowserItem::HistoryUp()
	{
		if (m_HistoryIndex >= m_pHistory.size() || m_pHistory.size() <= 1) return;
		++m_HistoryIndex;
		size_t index = m_pHistory.size() - m_HistoryIndex;
		m_pSelectedFolder = m_pHistory[index];
	}

	void FileBrowserItem::HistoryDown()
	{
		if (m_HistoryIndex <= 1 || m_pHistory.size() <= 1) return;
		--m_HistoryIndex;
		size_t index = m_pHistory.size() - m_HistoryIndex;
		m_pSelectedFolder = m_pHistory[index];
	}

	void FileBrowserItem::Change(const std::string& name, bool isFolder)
	{
		if (m_Name == name && m_IsFolder == isFolder) return;

		m_Name = name;
		m_IsFolder = isFolder;
		Refresh();
	}

	void FileBrowserItem::Refresh()
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
				m_pChildren.push_back(new FileBrowserItem(lastDir.string(), directory, this, m_Editable, m_DirectoryFilter, m_RootPathFunc));
				m_pChildren[childIndex]->AddIgnoreDirectories(m_IgnoreDirectories);
				FileBrowserItem* pNewChild = m_pChildren[childIndex];
				pNewChild->Refresh();
				pNewChild->SortChildren();
				continue;
			}

			m_pChildren[actualIndex]->Change(lastDir.string(), directory);
			m_pChildren[actualIndex]->Refresh();
			m_pChildren[actualIndex]->SortChildren();
		}

		std::vector<FileBrowserItem*>::iterator it = m_pChildren.begin() + index;
		if (it >= m_pChildren.end()) return;
		std::for_each(it, m_pChildren.end(), [](FileBrowserItem* pChild) { delete pChild; });
		m_pChildren.erase(it, m_pChildren.end());
	}

	void FileBrowserItem::RefreshSelected(FileBrowserItem* pRoot)
	{
		if (m_pSelectedFolder != nullptr && m_pSelectedFolder->IsValid()) return;
		m_pSelectedFolder = this;
		m_pHistory.clear();
		m_pHistory.push_back(pRoot);
		m_HistoryIndex = 1;
	}

	std::filesystem::path FileBrowserItem::BuildPath()
	{
		std::filesystem::path finalPath = "";
		if (m_pParent == nullptr)
			finalPath = m_RootPathFunc();
		else
			finalPath = m_pParent->m_CachedPath;

		finalPath.append(m_Name);
		return finalPath;
	}

	void FileBrowserItem::DrawDirectoryBrowser()
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

	void FileBrowserItem::DrawFileBrowser(int iconSize)
	{
		if (!m_pSelectedFolder) return;

		ImVec2 windowSize = ImGui::GetWindowSize();

		float width = windowSize.x;

		int columns = (int)(width / (iconSize + 32.0f));
		if (columns <= 0) columns = 1;

		for (size_t i = 0; i < m_pSelectedFolder->m_pChildren.size(); i++)
		{
			FileBrowserItem* pChild = m_pSelectedFolder->m_pChildren[i];

			const int columnIndex = (i % columns) - 1;
			ImGui::PushID(pChild->m_CachedPath.c_str());
			pChild->DrawFileItem(iconSize);
			ImGui::PopID();
			int mod = (i + 1) % columns;
			if (mod != 0) ImGui::SameLine();
		}
	}

	void FileBrowserItem::DrawCurrentPath()
	{
		if (!m_pSelectedFolder) return;

		std::vector<FileBrowserItem*> pPathTrace;
		pPathTrace.push_back(m_pSelectedFolder);
		FileBrowserItem* pParent = m_pSelectedFolder->m_pParent;
		while (pParent != nullptr)
		{
			pPathTrace.push_back(pParent);
			pParent = pParent->m_pParent;
		}

		std::reverse(pPathTrace.begin(), pPathTrace.end());
		for (size_t i = 0; i < pPathTrace.size(); i++)
		{
			FileBrowserItem* pItem = pPathTrace[i];
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

	void FileBrowserItem::DrawFileItem(int iconSize)
	{
		const ImVec4 buttonColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		const ImVec4 buttonInactiveColor = { buttonColor.x, buttonColor.y, buttonColor.z, 0.0f };

		const float padding = 10.0f;
		const float textHeight = ImGui::CalcTextSize("LABEL").y;
		const ImVec2 itemSize = { iconSize + padding * 2.0F, iconSize + padding * 3.0f + textHeight };

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::BeginChild("##file", itemSize, false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleVar(2);

		const ImVec2 cursorPos = ImGui::GetCursorPos();

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();
		if (m_IsFolder)
		{
			Texture* pFolderTexture = EditorAssets::GetTexture("folder");

			ImGui::PushStyleColor(ImGuiCol_Button, m_HighlightedPath == m_CachedPath.string() ? buttonColor : buttonInactiveColor);
			ImGui::Button("##fileItem", itemSize);
			ImGui::PopStyleColor();
			ImGui::SetItemAllowOverlap();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				m_pSelectedFolder = this;
				EraseExcessHistory();
				m_pHistory.push_back(this);
				SetOpen();
			}

			if (ImGui::IsItemClicked(0))
			{
				Selection::SetActiveObject(nullptr);
				m_HighlightedPath = m_CachedPath.string();
			}

			if (ImGui::IsItemClicked(1))
			{
				Selection::SetActiveObject(nullptr);
				m_HighlightedPath = m_CachedPath.string();
				ObjectMenu::Open(nullptr, m_Editable ? ObjectMenuType::T_Folder : ObjectMenuType::T_ModuleFolder);
			}

			ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
			ImGui::Image(pRenderImpl->GetTextureID(pFolderTexture), ImVec2((float)iconSize, (float)iconSize));

			DrawName(padding);
			ImGui::EndChild();
			return;
		}

		std::filesystem::path assetPath = ProjectSpace::GetOpenProject()->RootPath();
		assetPath.append("Assets");
		std::filesystem::path relativePath = m_CachedPath.lexically_relative(assetPath);
		if (relativePath == "") relativePath = m_CachedPath;
		UUID uuid = AssetDatabase::GetAssetUUID(relativePath.string());
		Texture* pTexture = Tumbnail::GetTumbnail(uuid);

		UUID selectedID = Selection::GetActiveObject() ? Selection::GetActiveObject()->GetUUID() : 0;

		ImGui::PushStyleColor(ImGuiCol_Button, selectedID == uuid || m_HighlightedPath == m_CachedPath.string() ? buttonColor : buttonInactiveColor);
		ImGui::Button("##fileItem", itemSize);
		ImGui::PopStyleColor();
		ImGui::SetItemAllowOverlap();

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			ResourceMeta meta;
			EditorAssetDatabase::GetAssetMetadata(uuid, meta);
			BaseTumbnailGenerator* pGenerator = Tumbnail::GetGenerator(meta.Hash());
			if (!pGenerator)
			{
				ImGui::EndChild();
				return;
			}
			pGenerator->OnFileDoubleClick(uuid);
		}

		if (ImGui::IsItemClicked(0))
		{
			m_HighlightedPath = m_CachedPath.string();
			Resource* pAsset = AssetManager::GetAssetImmediate(uuid);
			Selection::SetActiveObject(pAsset);
		}

		if (ImGui::IsItemClicked(1))
		{
			m_HighlightedPath = m_CachedPath.string();
			Resource* pAsset = AssetManager::GetAssetImmediate(uuid);
			ObjectMenu::Open(pAsset, m_Editable ? ObjectMenuType::T_Resource : ObjectMenuType::T_ModuleResource);
		}

		ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
		ImGui::Image(pTexture ? pRenderImpl->GetTextureID(pTexture) : NULL, ImVec2((float)iconSize, (float)iconSize));
		DrawName(padding);
		ImGui::EndChild();
	}

	bool FileBrowserItem::IsValid()
	{
		return std::filesystem::exists(m_CachedPath);
	}

	void FileBrowserItem::SetOpen()
	{
		if (m_pParent != nullptr) m_pParent->SetOpen();
		m_SetOpen = true;
	}

	void FileBrowserItem::SortChildren()
	{
		std::sort(m_pChildren.begin(), m_pChildren.end(), [&](FileBrowserItem* pA, FileBrowserItem* pB)
		{
			if (pA->m_IsFolder && !pB->m_IsFolder) return true;
			if (!pA->m_IsFolder && pB->m_IsFolder) return false;
			return pA->m_Name < pB->m_Name;
		});
	}

	std::filesystem::path FileBrowserItem::GetCurrentPath()
	{
		return m_pSelectedFolder != nullptr ? m_pSelectedFolder->BuildPath() : ProjectSpace::GetOpenProject()->RootPath();
	}

	FileBrowserItem* FileBrowserItem::GetChildByName(const std::string& name, bool folder)
	{
		auto it = std::find_if(m_pChildren.begin(), m_pChildren.end(), [&](FileBrowserItem* pChild) {return pChild->m_Name.find(name) != std::string::npos && pChild->m_IsFolder == folder; });
		if (it == m_pChildren.end()) return nullptr;
		return *it;
	}

	void FileBrowserItem::BeginRename()
	{
		m_StartEditingName = true;
		m_EditingName = true;
		std::string name = m_CachedPath.filename().replace_extension().string();
		memcpy(m_NameBuffer, name.data(), name.length());
		m_NameBuffer[name.length()] = '\0';
	}

	bool FileBrowserItem::IsEditable() const
	{
		return m_Editable;
	}

	const std::string& FileBrowserItem::GetHighlightedPath()
	{
		return m_HighlightedPath;
	}

	void FileBrowserItem::AddIgnoreDirectory(const std::string& directory)
	{
		m_IgnoreDirectories.push_back(directory);
	}

	void FileBrowserItem::AddIgnoreDirectories(const std::vector<std::string>& directories)
	{
		m_IgnoreDirectories = directories;
	}

	const std::string& FileBrowserItem::Name()
	{
		return m_Name;
	}

	void FileBrowserItem::EraseExcessHistory()
	{
		if (m_HistoryIndex <= 1) return;
		std::vector<FileBrowserItem*>::iterator it = m_pHistory.end() - (m_HistoryIndex - 1);
		if (it >= m_pHistory.end()) return;
		m_pHistory.erase(it, m_pHistory.end());
		m_HistoryIndex = 1;
	}

	void FileBrowserItem::DrawName(float padding)
	{
		const ImVec2 cursorPos = ImGui::GetCursorPos();

		const float availableWidth = ImGui::GetContentRegionAvail().x - padding;
		const std::string text = m_CachedPath.filename().replace_extension().string();
		const float textWidth = ImGui::CalcTextSize(text.data()).x;
		const int wraps = textWidth / (availableWidth - 8.0f);

		ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y - wraps * padding / 1.2f });

		if (m_EditingName || m_StartEditingName)
		{
			ImGui::SetNextItemWidth(availableWidth - padding);
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

				std::filesystem::path assetPath = Game::GetAssetPath();
				EditorAssetDatabase::UpdateAssetPaths(m_CachedPath.lexically_relative(assetPath).string(), newPath.lexically_relative(assetPath).string());
				m_CachedPath = newPath;
				Refresh();
			}
			m_StartEditingName = false;
		}
		else
		{
			ImGui::PushTextWrapPos(availableWidth);
			ImGui::TextWrapped(text.data());
			ImGui::PopTextWrapPos();
		}
	}

	std::filesystem::path FileBrowserItem::DefaultRootPathFunc()
	{
		std::string projectRootPath = "";
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		if (pProject != nullptr) projectRootPath = pProject->RootPath();
		return projectRootPath;
	}
}
