#include "ScriptTypeReferenceDrawer.h"

#include <PropertyDrawer.h>
#include <EditorApplication.h>
#include <EditorUI.h>
#include <DND.h>
#include <EditorAssetDatabase.h>
#include <EditorAssetManager.h>

#include <GloryMonoScipting.h>
#include <WindowModule.h>
#include <MonoManager.h>
#include <CoreLibManager.h>

#include <StringUtils.h>

namespace Glory::Editor
{
	constexpr std::string_view None = "None";

	bool ForceFilter = false;

	template<>
	inline bool PropertyDrawerTemplate<ScriptTypeReference>::OnGUI(const std::string& label, ScriptTypeReference* data, uint32_t flags) const
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		GloryMonoScipting* pScripting = pApp->GetEngine()->GetOptionalModule<GloryMonoScipting>();
		EditorAssetManager& assets = pApp->GetAssetManager();
		const MonoScriptManager& scriptManager = pScripting->GetMonoManager()->GetCoreLibManager()->ScriptManager();
		const int typeIndex = scriptManager.TypeIndexFromHash(data->m_Hash);
		size_t index = typeIndex + 1;

		bool openPopup = false;
		float start, width;
		EditorUI::EmptyDropdown(EditorUI::MakeCleanName(label), index == 0 ? None : scriptManager.TypeName(index - 1), [&openPopup]() {
			openPopup = true;
			ForceFilter = true;
		}, start, width, 0.0f);

		static uint32_t scriptTypeHash = ResourceTypes::GetHash<MonoScript>();

		bool change = false;
		change = DND{ { ST_Path, scriptTypeHash } }.HandleDragAndDropTarget([&](uint32_t type, const ImGuiPayload* payload)
		{
			UUID uuid = 0;

			if (type == ST_Path)
			{
				const std::string path = (const char*)payload->Data;
				uuid = EditorAssetDatabase::FindAssetUUID(path);
				if (!uuid) return;
				ResourceMeta meta;
				if (!EditorAssetDatabase::GetAssetMetadata(uuid, meta)) return;
				if (meta.Hash() != scriptTypeHash) return;
			}
			else uuid = *(const UUID*)payload->Data;

			if (uuid == 0) return;
			Resource* pResource = assets.GetAssetImmediate(uuid);
			if (!pResource) return;
			MonoScript* pScript = static_cast<MonoScript*>(pResource);
			for (size_t i = 0; i < pScript->ClassCount(); i++)
			{
				const std::string_view typeName = pScript->ClassName(i);
				const int typeIndex = scriptManager.TypeIndexFromName(typeName);
				if (typeIndex == -1) continue;
				index = typeIndex + 1;
				return;
			}
		});

		if (openPopup)
			ImGui::OpenPopup("ScriptDropdown");

		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		Window* pWindow = EditorApplication::GetInstance()->GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
		int mainWindowWidth, mainWindowHeight;
		pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
		ImGui::SetNextWindowPos({ windowPos.x + start, windowPos.y + cursor.y - 2.5f });
		ImGui::SetNextWindowSize({ width, mainWindowHeight - windowPos.y - cursor.y - 10.0f });

		if (ImGui::BeginPopup("ScriptDropdown"))
		{
			static char searchBuffer[200] = "";
			static std::vector<uint32_t> searchResultCache;

			const bool needsFilter = EditorUI::SearchBar(ImGui::GetContentRegionAvail().x, searchBuffer, 200) || ForceFilter;
			ForceFilter = false;
			if (needsFilter)
			{
				searchResultCache.clear();
				const std::string_view search = searchBuffer;
				for (size_t i = 0; i < scriptManager.TypeCount(); ++i)
				{
					const std::string_view typeName = scriptManager.TypeName(i);
					if (!search.empty() && Utils::CaseInsensitiveSearch(typeName, search) == std::string::npos) continue;
					searchResultCache.push_back(scriptManager.TypeHash(i));
				}
			}

			if (ImGui::Selectable("None", index == 0))
			{
				index = 0;
				change = true;
			}

			ImGui::BeginChild("scrollregion");
			const float rowHeight = 22.0f;
			ImGuiListClipper clipper{ int(searchResultCache.size()), rowHeight };

			auto itorStart = searchResultCache.begin();
			while (clipper.Step()) {
				const auto start = itorStart + clipper.DisplayStart;
				const auto end = itorStart + clipper.DisplayEnd;

				for (auto it = start; it != end; ++it)
				{
					const uint32_t hash = *it;
					ImGui::PushID(hash);
					const int otherTypeIndex = scriptManager.TypeIndexFromHash(hash);
					const std::string_view typeName = scriptManager.TypeName(otherTypeIndex);
					if (ImGui::Selectable("##select", typeIndex == otherTypeIndex, ImGuiSelectableFlags_AllowItemOverlap, { 0.0f, rowHeight }))
					{
						index = otherTypeIndex + 1;
						change = true;
					}

					ImGui::SameLine();
					ImGui::TextUnformatted(typeName.data());
					ImGui::PopID();
				}
			}
			ImGui::EndChild();

			if (change)
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		if (change)
			data->m_Hash = index == 0 ? 0 : scriptManager.TypeHash(index - 1);

		return change;
	}
}

namespace YAML
{
	Emitter& YAML::operator<<(Emitter& out, const Glory::ScriptTypeReference& ref)
	{
		out << ref.m_Hash;
		return out;
	}
}
