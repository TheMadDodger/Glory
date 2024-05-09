#include "ResourcesWindow.h"
#include "EditorAssetDatabase.h"
#include "Tumbnail.h"
#include "EditorApplication.h"
#include "EditorAssetManager.h"

#include <EditorUI.h>
#include <StringUtils.h>
#include <ProjectSpace.h>
#include <EditorAssetCallbacks.h>

#include <PrefabData.h>
#include <ResourceType.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	const size_t SearchBufferSize = 1000;
	char SearchBuffer[SearchBufferSize] = "\0";
	std::vector<ResourceType*> AllResourceTypes;
	size_t TabBarIndex = 1;
	bool FirstGui = true;

	enum FilterOption : size_t
	{
		FO_All,
		FO_Unloaded,
		FO_Loaded,
		FO_Loading,
		FO_Count
	};

	constexpr std::string_view FilterNames[] = {
		"Show All",
		"Show Unloaded",
		"Show Loaded",
		"Show Loading",
	};

	FilterOption Filter = FilterOption::FO_All;

	bool ResourcesWindow::m_ForceFilter = true;

	ResourcesWindow::ResourcesWindow() : EditorWindowTemplate("Resources", 600.0f, 600.0f),
		m_ProjectOpenCallback(0),
		m_AssetRegisteredCallback(0),
		m_AssetDeletedCallback(0)
	{
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

			if (ImGui::BeginTabItem("All", nullptr, FirstGui ? ImGuiTabItemFlags_SetSelected : 0))
			{
				if (TabBarIndex != 1)
				{
					TabBarIndex = 1;
					m_ForceFilter = true;
				}
				ImGui::EndTabItem();
			}

			FirstGui = false;

			for (size_t i = 0; i < AllResourceTypes.size(); ++i)
			{
				const std::string& name = AllResourceTypes[i]->Name();
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

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		static const ImGuiTableFlags flags =
			ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg
			| ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;

		const float rowHeight = 64.0f;

		const float regionWidth = ImGui::GetWindowContentRegionMax().x;
		bool needsFilter = TabBarIndex == 0 || EditorUI::SearchBar(regionWidth, SearchBuffer, SearchBufferSize) || m_ForceFilter;
		m_ForceFilter = false;

		if (TabBarIndex != 0)
		{
			ImGui::TextUnformatted(ICON_FA_FILTER " Filter");
			for (size_t i = 0; i < FilterOption::FO_Count; ++i)
			{
				FilterOption filter = FilterOption(i);
				ImGui::SameLine();
				if (ImGui::RadioButton(FilterNames[i].data(), Filter == filter))
				{
					Filter = filter;
					needsFilter = true;
				}
			}
		}

		if (needsFilter)
			RunFilter();

		if (!ImGui::BeginChild("ResourcesChild") || !ImGui::BeginTable("ResourcesTable", 8, flags))
		{
			ImGui::EndChild();
			return;
		}

		ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 40.0f, 1);
		ImGui::TableSetupColumn("Thumb", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, rowHeight, 1);
		ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f, 2);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 110.0f, 3);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.2f, 4);
		ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 500.0f, 5);
		ImGui::TableSetupColumn("Sub Path", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.7f, 6);
		ImGui::TableSetupColumn("Loaded?", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, rowHeight, 7);

		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableHeadersRow();

		ImGuiListClipper clipper(m_SearchResultCache.size(), rowHeight + 2*ImGui::GetCurrentTable()->CellPaddingY);

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorAssetManager& assetManager = EditorApplication::GetInstance()->GetAssetManager();
		ResourceTypes& resourceTypes = pEngine->GetResourceTypes();

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

				ResourceMeta meta;
				EditorAssetDatabase::GetAssetMetadata(uuid, meta);
				const ResourceType* pType = resourceTypes.GetResourceType(meta.Hash());
				Texture* pThumbnail = Tumbnail::GetTumbnail(uuid);
				const std::string name = EditorAssetDatabase::GetAssetName(uuid);

				AssetLocation location;
				EditorAssetDatabase::GetAssetLocation(uuid, location);

				if (ImGui::TableNextColumn())
				{
					ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

					if (ImGui::Selectable("##selectable", false, selectableFlags, ImVec2(0, rowHeight)))
					{
						/* Select it? */
					}

					AssetPayload payload{ uuid };
					const uint32_t subTypeHash = resourceTypes.GetSubTypeHash(pType, 1);
					const ResourceType* pPayloadType = pType;
					if (subTypeHash != ResourceTypes::GetHash<Resource>())
					{
						pPayloadType = resourceTypes.GetResourceType(subTypeHash);
						if (pPayloadType == nullptr) pPayloadType = pType;
					}

					DND::DragAndDropSource(pPayloadType->Name(), &payload, sizeof(AssetPayload), [&]() {
						ImGui::Image(pThumbnail ? pRenderImpl->GetTextureID(pThumbnail) : NULL, { 64.0f, 64.0f });
						ImGui::SameLine();
						ImGui::Text(name.data());
					});

					ImGui::SameLine();
					ImGui::Text("%i", index);
				}

				if (ImGui::TableNextColumn())
				{
					ImGui::Image(pThumbnail ? pRenderImpl->GetTextureID(pThumbnail) : NULL, { rowHeight, rowHeight });
				}

				if (ImGui::TableNextColumn())
				{
					ImGui::Text("%s", std::to_string(uuid).data());
				}

				if (ImGui::TableNextColumn())
				{
					ImGui::Text("%s", pType->Name().data());
				}

				if (ImGui::TableNextColumn())
				{
					ImGui::Text("%s", name.data());
				}

				if (ImGui::TableNextColumn())
				{
					ImGui::Text("%s", location.Path.data());
				}

				if (ImGui::TableNextColumn())
				{
					ImGui::Text("%s", location.SubresourcePath.data());
				}

				if (ImGui::TableNextColumn())
				{
					const bool loaded = assetManager.FindResource(uuid) != nullptr;
					ImGui::Text("%s", loaded ? "Yes" : assetManager.IsLoading(uuid) ? "Loading..." : "No");
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

		m_ProjectOpenCallback = ProjectSpace::RegisterCallback(ProjectCallback::OnOpen, [&](ProjectSpace*) { m_ForceFilter = true; });
		AllResourceTypes.clear();
		std::vector<ResourceType*> types;
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		ResourceTypes& resourceTypes = pEngine->GetResourceTypes();

		resourceTypes.GetAllResourceTypesThatHaveSubType(ResourceTypes::GetHash<Resource>(), types);
		for (size_t i = 0; i < types.size(); ++i)
		{
			ResourceType* pType = types[i];
			AllResourceTypes.push_back(pType);
		}

		m_AssetRegisteredCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered,
			[&](const AssetCallbackData&) { m_ForceFilter = true; });
		m_AssetDeletedCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetDeleted,
			[&](const AssetCallbackData&) { m_ForceFilter = true; });
	}

	void ResourcesWindow::OnClose()
	{
		ProjectSpace::RemoveCallback(ProjectCallback::OnOpen, m_ProjectOpenCallback);
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetRegisteredCallback);
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetDeleted, m_AssetDeletedCallback);
	}

	void ResourcesWindow::RunFilter()
	{
		m_SearchResultCache.clear();
		m_SearchResultIndexCache.clear();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorAssetManager& assetManager = EditorApplication::GetInstance()->GetAssetManager();
		ResourceTypes& resourceTypes = pEngine->GetResourceTypes();

		if (TabBarIndex == 0)
		{
			assetManager.GetAllLoading(m_SearchResultCache);
			return;
		}

		std::string_view search{SearchBuffer};
		const std::vector<UUID> allResources = EditorAssetDatabase::UUIDs();
		for (size_t i = 0; i < allResources.size(); ++i)
		{
			const std::string name = EditorAssetDatabase::GetAssetName(allResources[i]);
			if (TabBarIndex != 1)
			{
				const ResourceType* pFilteredType = AllResourceTypes[TabBarIndex - 2];
				ResourceMeta meta;
				EditorAssetDatabase::GetAssetMetadata(allResources[i], meta);
				const ResourceType* pType = resourceTypes.GetResourceType(meta.Hash());
				if (pType != pFilteredType) continue;
			}

			const bool loaded = assetManager.FindResource(allResources[i]) != nullptr;
			const bool loading = assetManager.IsLoading(allResources[i]);

			switch (Filter)
			{
			case Glory::Editor::FO_Unloaded:
				if (loaded) continue;
				break;
			case Glory::Editor::FO_Loaded:
				if (!loaded) continue;
				break;
			case Glory::Editor::FO_Loading:
				if (!loading) continue;
				break;
			default:
				break;
			}

			if (!search.empty() && Utils::CaseInsensitiveSearch(name, search) == std::string::npos) continue;
			m_SearchResultCache.push_back(allResources[i]);
			m_SearchResultIndexCache.push_back(i);
		}
	}
}
