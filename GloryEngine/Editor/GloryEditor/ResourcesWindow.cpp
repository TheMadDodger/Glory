#include "ResourcesWindow.h"
#include "EditorAssetDatabase.h"
#include "Tumbnail.h"
#include "EditorApplication.h"

#include <AssetManager.h>
#include <EditorUI.h>
#include <StringUtils.h>
#include <ProjectSpace.h>
#include <EditorAssetCallbacks.h>

namespace Glory::Editor
{
	const size_t SearchBufferSize = 1000;
	char SearchBuffer[SearchBufferSize] = "\0";
	std::vector<ResourceType*> ResourceTypes;
	size_t TabBarIndex = 1;

	bool ResourcesWindow::m_ForceFilter = true;
	bool ResourcesWindow::m_SubscribedToEvents = false;

	ResourcesWindow::ResourcesWindow() : EditorWindowTemplate("Resources", 600.0f, 600.0f)
	{
		if (m_SubscribedToEvents) return;
		ProjectSpace::RegisterCallback(ProjectCallback::OnOpen, [&](ProjectSpace*) { m_ForceFilter = true; });
		ResourceTypes.clear();
		ResourceType::GetAllResourceTypesThatHaveSubType(ResourceType::GetHash<Resource>(), ResourceTypes);

		EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered, [&](UUID, const ResourceMeta&, Resource*) { m_ForceFilter = true; });
		EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetDeleted, [&](UUID, const ResourceMeta&, Resource*) { m_ForceFilter = true; });
	}

	ResourcesWindow::~ResourcesWindow()
	{
	}

	void ResourcesWindow::OnGUI()
	{
		if (ImGui::BeginTabBar("ResourcesTabs"))
		{
			if (ImGui::BeginTabItem("Loading"))
			{
				if (TabBarIndex != 0)
				{
					TabBarIndex = 0;
					m_ForceFilter = true;
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("All"))
			{
				if (TabBarIndex != 1)
				{
					TabBarIndex = 1;
					m_ForceFilter = true;
				}
				ImGui::EndTabItem();
			}

			for (size_t i = 0; i < ResourceTypes.size(); ++i)
			{
				const std::string& name = ResourceTypes[i]->Name();
				if (ImGui::BeginTabItem(name.data()))
				{
					if (TabBarIndex != i + 2)
					{
						TabBarIndex = i + 2;
						m_ForceFilter = true;
					}
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform()->GetRenderImpl();

		static const ImGuiTableFlags flags =
			ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg
			| ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;

		const float rowHeight = 64.0f;

		const float regionWidth = ImGui::GetWindowContentRegionMax().x;
		const bool needsFilter = TabBarIndex == 0 || EditorUI::SearchBar(regionWidth, SearchBuffer, SearchBufferSize) || m_ForceFilter;
		m_ForceFilter = false;

		if (needsFilter)
			RunFilter();

		if (!ImGui::BeginChild("ResourcesChild") || !ImGui::BeginTable("ResourcesTable", 6, flags))
		{
			ImGui::EndChild();
			return;
		}

		ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, rowHeight, 1);
		ImGui::TableSetupColumn("Thumb", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, rowHeight, 1);
		ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f, 2);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 110.0f, 3);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.3f, 4);
		ImGui::TableSetupColumn("Loaded?", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.1f, 5);

		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableHeadersRow();

		ImGuiListClipper clipper(m_SearchResultCache.size(), rowHeight + 2*ImGui::GetCurrentTable()->CellPaddingY);

		auto itorStart = m_SearchResultCache.begin();
		while (clipper.Step()) {
			const auto start = itorStart + clipper.DisplayStart;
			const auto end = itorStart + clipper.DisplayEnd;

			for (auto it = start; it != end; ++it) {
				const UUID uuid = *it;
				const size_t searchResultIndex = m_SearchResultCache.size() - (m_SearchResultCache.end() - it);

				const size_t index = m_SearchResultIndexCache.empty()
					? searchResultIndex : m_SearchResultIndexCache[searchResultIndex];

				ImGui::PushID(uuid);
				ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

				if (ImGui::TableNextColumn())
				{
					ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

					if (ImGui::Selectable("##selectable", false, selectableFlags, ImVec2(0, rowHeight)))
					{
						/* Select it? */
					}

					ImGui::SameLine();
					ImGui::Text("%i", index);
				}

				if (ImGui::TableNextColumn())
				{
					Texture* pThumbnail = Tumbnail::GetTumbnail(uuid);
					ImGui::Image(pThumbnail ? pRenderImpl->GetTextureID(pThumbnail) : NULL, { rowHeight, rowHeight });
				}

				if (ImGui::TableNextColumn())
				{
					ImGui::Text("%s", std::to_string(uuid).data());
				}

				if (ImGui::TableNextColumn())
				{
					ResourceMeta meta;
					EditorAssetDatabase::GetAssetMetadata(uuid, meta);
					const ResourceType* pType = ResourceType::GetResourceType(meta.Hash());
					ImGui::Text("%s", pType->Name().data());
				}

				if (ImGui::TableNextColumn())
				{
					const std::string name = EditorAssetDatabase::GetAssetName(uuid);
					ImGui::Text("%s", name.data());
				}

				if (ImGui::TableNextColumn())
				{
					const bool loaded = AssetManager::FindResource(uuid) != nullptr;
					ImGui::Text("%s", loaded ? "Yes" : AssetManager::IsLoading(uuid) ? "Loading..." : "No");
				}

				ImGui::PopID();
			}
		}

		ImGui::EndTable();
		ImGui::EndChild();
	}

	void ResourcesWindow::OnOpen()
	{
		m_ForceFilter = true;
	}
	void ResourcesWindow::RunFilter()
	{
		m_SearchResultCache.clear();
		m_SearchResultIndexCache.clear();

		if (TabBarIndex == 0)
		{
			AssetManager::GetAllLoading(m_SearchResultCache);
			return;
		}

		std::string_view search{SearchBuffer};
		const std::vector<UUID> allResources = EditorAssetDatabase::UUIDs();
		for (size_t i = 0; i < allResources.size(); ++i)
		{
			const std::string name = EditorAssetDatabase::GetAssetName(allResources[i]);
			if (TabBarIndex != 1)
			{
				const ResourceType* pFilteredType = ResourceTypes[TabBarIndex - 2];
				ResourceMeta meta;
				EditorAssetDatabase::GetAssetMetadata(allResources[i], meta);
				const ResourceType* pType = ResourceType::GetResourceType(meta.Hash());
				if (pType != pFilteredType) continue;
			}

			if (!search.empty() && Utils::CaseInsensitiveSearch(name, search) == std::string::npos) continue;
			m_SearchResultCache.push_back(allResources[i]);
			m_SearchResultIndexCache.push_back(i);
		}
	}
}
