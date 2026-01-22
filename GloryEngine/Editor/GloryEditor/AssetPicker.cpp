#include "AssetPicker.h"
#include "EditorUI.h"
#include "DND.h"
#include "Tumbnail.h"
#include "EditorApplication.h"
#include "FileBrowser.h"

#include <imgui.h>
#include <algorithm>
#include <EditorAssetDatabase.h>
#include <Engine.h>
#include <WindowModule.h>
#include <SerializedTypes.h>
#include <StringUtils.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	char AssetPicker::m_SearchBuffer[200] = "";
	std::vector<UUID> AssetPicker::m_SearchResultCache;

	bool ForceFilter = false;

	bool AssetPicker::ResourceDropdown(const std::string& label, uint32_t resourceType, UUID* value, bool includeSubAssets, const float borderPadding)
	{
		ImGui::PushID(label.c_str());
		std::string assetName = "";
		assetName = EditorAssetDatabase::GetAssetName(*value);
		if (assetName == "") assetName = "Noone";

		ResourceTypes& resourceTypes = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes();

		bool openPopup = false;
		float start, width;
		EditorUI::EmptyDropdown(EditorUI::MakeCleanName(label), assetName, [&]
		{
			ForceFilter = true;
			openPopup = true;
		}, start, width, borderPadding);

		bool change = false;
		if (resourceType)
		{
			change = DND{ { ST_Path, resourceType } }.HandleDragAndDropTarget([&](uint32_t type, const ImGuiPayload* payload)
			{
				if (type == ST_Path)
				{
					const std::string path = (const char*)payload->Data;
					const UUID uuid = EditorAssetDatabase::FindAssetUUID(path);
					if (!uuid) return;
					ResourceMeta meta;
					if (!EditorAssetDatabase::GetAssetMetadata(uuid, meta)) return;
					ResourceType* pResourceType = resourceTypes.GetResourceType(meta.Hash());

					if (meta.Hash() == resourceType)
					{
						*value = uuid;
						return;
					}

					for (size_t i = 0; i < resourceTypes.SubTypeCount(pResourceType); ++i)
					{
						if (resourceTypes.GetSubTypeHash(pResourceType, i) != resourceType) continue;
						*value = uuid;
					}
					return;
				}

				*value = *(const UUID*)payload->Data;
			});
		}

		if (openPopup)
			ImGui::OpenPopup("AssetPicker");
		openPopup = false;

		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		Window* pWindow = EditorApplication::GetInstance()->GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
		int mainWindowWidth, mainWindowHeight;
		pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
		ImGui::SetNextWindowPos({ windowPos.x + start, windowPos.y + cursor.y - 2.5f });
		ImGui::SetNextWindowSize({ width, mainWindowHeight - windowPos.y - cursor.y - 10.0f });
		change |= DrawPopup(value, resourceType, includeSubAssets);
		ImGui::PopID();
		return change;
	}

	bool AssetPicker::ResourceButton(const std::string& label, float buttonWidth, uint32_t resourceType, UUID* value, bool includeSubAssets)
	{
		ImGui::PushID(label.c_str());
		bool openPopup = false;
		if (ImGui::Button(label.c_str(), ImVec2(buttonWidth, 0.0f)))
		{
			ForceFilter = true;
			openPopup = true;
		}
		if (openPopup)
		{
			ImGui::OpenPopup("AssetPicker");
		}
		openPopup = false;

		Window* pWindow = EditorApplication::GetInstance()->GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
		int mainWindowWidth, mainWindowHeight;
		pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		ImGui::SetNextWindowPos({ windowPos.x + ImGui::GetCursorPosX(), windowPos.y + cursor.y - 2.5f});
		ImGui::SetNextWindowSize({ buttonWidth, mainWindowHeight - windowPos.y - cursor.y - 10.0f });
		const bool change = DrawPopup(value, resourceType, includeSubAssets);
		ImGui::PopID();
		return change;
	}

	bool AssetPicker::ResourceTumbnailButton(const std::string& label, float buttonWidth, float popupStart, float popupWidth, uint32_t resourceType, UUID* value, bool includeSubAssets)
	{
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		ImGui::PushID(label.c_str());
		std::string assetName = "";
		assetName = EditorAssetDatabase::GetAssetName(*value);
		if (assetName == "") assetName = "Noone";

		ResourceTypes& resourceTypes = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes();

		bool openPopup = false;
		const float start = popupStart, width = popupWidth;
		TextureHandle thumbnail = Tumbnail::GetTumbnail(*value);
		if(*value && ImGui::ImageButton(thumbnail ? pRenderImpl->GetTextureID(thumbnail) : NULL, ImVec2(buttonWidth, buttonWidth)))
		{
			ForceFilter = true;
			openPopup = true;
		}
		if (!*value && ImGui::Button(ICON_FA_CIRCLE_XMARK, ImVec2(buttonWidth*1.4f, buttonWidth*1.4f)))
		{
			ForceFilter = true;
			openPopup = true;
		}

		bool change = false;
		if (resourceType)
		{
			change = DND{ { ST_Path, resourceType } }.HandleDragAndDropTarget([&](uint32_t type, const ImGuiPayload* payload)
			{
				if (type == ST_Path)
				{
					const std::string path = (const char*)payload->Data;
					const UUID uuid = EditorAssetDatabase::FindAssetUUID(path);
					if (!uuid) return;
					ResourceMeta meta;
					if (!EditorAssetDatabase::GetAssetMetadata(uuid, meta)) return;
					ResourceType* pResourceType = resourceTypes.GetResourceType(meta.Hash());

					if (meta.Hash() == resourceType)
					{
						*value = uuid;
						return;
					}

					for (size_t i = 0; i < resourceTypes.SubTypeCount(pResourceType); ++i)
					{
						if (resourceTypes.GetSubTypeHash(pResourceType, i) != resourceType) continue;
						*value = uuid;
					}
					return;
				}

				*value = *(const UUID*)payload->Data;
			});
		}

		if (openPopup)
			ImGui::OpenPopup("AssetPicker");
		openPopup = false;

		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		Window* pWindow = EditorApplication::GetInstance()->GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
		int mainWindowWidth, mainWindowHeight;
		pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
		ImGui::SetNextWindowPos({ windowPos.x + start, windowPos.y + cursor.y - 2.5f });
		ImGui::SetNextWindowSize({ width, mainWindowHeight - windowPos.y - cursor.y - 10.0f });
		change |= DrawPopup(value, resourceType, includeSubAssets);
		ImGui::PopID();
		return change;
	}

	void AssetPicker::LoadAssets(uint32_t typeHash, bool includeSubAssets)
	{
		std::vector<UUID> assets;
		if (!typeHash)
		{
			assets = EditorAssetDatabase::UUIDs();
		}
		else
		{
			EditorAssetDatabase::GetAllAssetsOfType(typeHash, assets);

			if (includeSubAssets)
			{
				std::vector<ResourceType*> pTypes;
				EditorApplication::GetInstance()->GetEngine()->GetResourceTypes().GetAllResourceTypesThatHaveSubType(typeHash, pTypes);
				for (size_t i = 0; i < pTypes.size(); i++)
				{
					if (pTypes[i]->Hash() == typeHash) continue;
					EditorAssetDatabase::GetAllAssetsOfType(pTypes[i]->Hash(), assets);
				}
			}
		}

		const std::string_view search{m_SearchBuffer};
		m_SearchResultCache.clear();

		if (search.empty())
		{
			m_SearchResultCache.resize(assets.size());
			std::memcpy(m_SearchResultCache.data(), assets.data(), sizeof(UUID)*assets.size());
			return;
		}

		for (size_t i = 0; i < assets.size(); i++)
		{
			const UUID uuid = assets[i];
			const std::string name = EditorAssetDatabase::GetAssetName(uuid);
			if (Utils::CaseInsensitiveSearch(name, search) == std::string::npos) continue;
			m_SearchResultCache.push_back(assets[i]);
		}
	}

	bool AssetPicker::DrawPopup(UUID* value, uint32_t typeHash, bool includeSubAssets)
	{
		bool assetChosen = false;

		if (ImGui::BeginPopup("AssetPicker"))
		{
			const bool needsFilter = EditorUI::SearchBar(ImGui::GetContentRegionAvail().x, m_SearchBuffer, 200) || ForceFilter;
			ForceFilter = false;
			if (needsFilter)
				LoadAssets(typeHash, includeSubAssets);

			assetChosen = DrawItems(m_SearchResultCache, value);
			if (assetChosen)
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		return assetChosen;
	}

	bool AssetPicker::DrawItems(const std::vector<UUID>& items, UUID* value)
	{
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		bool change = false;
		if (ImGui::Selectable("None", *value == 0))
		{
			*value = 0;
			change = true;
		}

		ImGui::BeginChild("scrollregion");
		const float rowHeight = 64.0f;
		ImGuiListClipper clipper{ int(items.size()), rowHeight };

		auto itorStart = items.begin();
		while (clipper.Step()) {
			const auto start = itorStart + clipper.DisplayStart;
			const auto end = itorStart + clipper.DisplayEnd;

			for (auto it = start; it != end; ++it)
			{
				ImGui::PushID(*it);
				const std::string name = EditorAssetDatabase::GetAssetName(*it);
				if (ImGui::Selectable("##select", *it == *value, ImGuiSelectableFlags_AllowItemOverlap, { 0.0f, rowHeight }))
				{
					*value = *it;
					change = true;
				}
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					AssetLocation location;
					if (EditorAssetDatabase::GetAssetLocation(*it, location))
						FileBrowser::NavigateToAndHighlight(location.Path);
				}

				TextureHandle thumbnail = Tumbnail::GetTumbnail(*it);
				ImGui::SameLine();
				ImGui::Image(thumbnail ? pRenderImpl->GetTextureID(thumbnail) : NULL, { rowHeight, rowHeight });
				ImGui::SameLine();
				ImGui::TextUnformatted(name.c_str());
				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		return change;
	}
}