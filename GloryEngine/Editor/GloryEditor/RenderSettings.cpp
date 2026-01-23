#include "ProjectSettings.h"
#include "EditorUI.h"
#include "SettingsEnums.h"
#include "ListView.h"
#include "AssetPicker.h"
#include "EditorAssetDatabase.h"
#include "EditorPipelineManager.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"
#include "EditorApplication.h"
#include "AssetCompiler.h"
#include "Selection.h"

#include <PipelineData.h>

#include <imgui.h>
#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	std::vector<UUID> IncludedPipelines;

	bool RenderSettings::OnGui()
	{
		ImGui::BeginChild("Render Settings");
		bool change = false;

		if (ImGui::CollapsingHeader("Pipeline Order"))
		{
			auto pipelineOrder = RootValue()["PipelineOrder"];
			if (!pipelineOrder.Exists())
				pipelineOrder.Set(YAML::Node(YAML::NodeType::Sequence));

			static const ImGuiTableFlags flags =
				ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg
				| ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
			if (!ImGui::BeginChild("ResourcesChild") || !ImGui::BeginTable("ResourcesTable", 4, flags))
			{
				ImGui::EndChild();
				return false;
			}

			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 40.0f, 0);
			ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 100.0f, 1);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.2f, 2);
			ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 500.0f, 3);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			const float rowHeight = 24.0f;

			ImGuiListClipper clipper(pipelineOrder.Size(), rowHeight + 2 * ImGui::GetCurrentTable()->CellPaddingY);

			Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
			EditorAssetManager& assetManager = EditorApplication::GetInstance()->GetAssetManager();
			ResourceTypes& resourceTypes = pEngine->GetResourceTypes();

			while (clipper.Step())
			{
				for (size_t i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
				{
					const UUID uuid = pipelineOrder[i].As<uint64_t>();
					//const size_t searchResultIndex = m_SearchResultCache.size() - (m_SearchResultCache.end() - it);

					//const size_t index = m_SearchResultIndexCache.empty()
						//? searchResultIndex : m_SearchResultIndexCache[searchResultIndex];

					ImGui::PushID(uuid);
					ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

					ResourceMeta meta;
					EditorAssetDatabase::GetAssetMetadata(uuid, meta);
					const std::string name = EditorAssetDatabase::GetAssetName(uuid);

					AssetLocation location;
					EditorAssetDatabase::GetAssetLocation(uuid, location);

					if (ImGui::TableNextColumn())
					{
						ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

						if (ImGui::Selectable("##selectable", false, selectableFlags, ImVec2(0, rowHeight)))
						{
							Resource* pResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(uuid);
							if (!pResource) pResource = EditorApplication::GetInstance()->GetPipelineManager().GetPipelineData(uuid);
							Selection::SetActiveObject(pResource);
						}

						//AssetPayload payload{ uuid };
						//const uint32_t subTypeHash = pType->Hash();//resourceTypes.GetSubTypeHash(pType, 1);
						//const ResourceType* pPayloadType = pType;
						//if (subTypeHash != ResourceTypes::GetHash<Resource>() &&
						//	subTypeHash != ResourceTypes::GetHash<GScene>())
						//{
						//	pPayloadType = resourceTypes.GetResourceType(subTypeHash);
						//	if (pPayloadType == nullptr) pPayloadType = pType;
						//}

						//DND::DragAndDropSource(pPayloadType->Name(), &payload, sizeof(AssetPayload), [&]() {
						//	ImGui::Image(thumbnail ? pRenderImpl->GetTextureID(thumbnail) : NULL, { 64.0f, 64.0f });
						//	ImGui::SameLine();
						//	ImGui::Text(name.data());
						//});

						ImGui::SameLine();
						ImGui::Text("%i", i);
					}

					if (ImGui::TableNextColumn())
					{
						ImGui::Text("%s", std::to_string(uuid).data());
					}

					if (ImGui::TableNextColumn())
					{
						ImGui::Text("%s", name.data());
					}

					if (ImGui::TableNextColumn())
					{
						ImGui::Text("%s", location.Path.data());
					}

					ImGui::PopID();
				}
			}

			ImGui::EndTable();
			ImGui::EndChild();
		}

		ImGui::EndChild();

		return false;
	}

	void RenderSettings::OnOpen()
	{
		AddNewPipelines();
	}

	void RenderSettings::OnSettingsLoaded()
	{
		VerifySettings();
	}

	void RenderSettings::AddNewPipelines()
	{
		auto pipelineOrder = RootValue()["PipelineOrder"];

		static const uint32_t pipelineHash = ResourceTypes::GetHash<PipelineData>();

		std::vector<UUID> allPipelines;
		EditorAssetDatabase::GetAllAssetsOfType(pipelineHash, allPipelines);
		for (auto pipelineID : allPipelines)
		{
			auto iter = std::find(IncludedPipelines.begin(), IncludedPipelines.end(), pipelineID);
			if (iter != IncludedPipelines.end()) continue;
			pipelineOrder.PushBack(uint64_t(pipelineID));
			IncludedPipelines.emplace_back(pipelineID);
		}
	}

	void RenderSettings::VerifySettings()
	{
		auto pipelineOrder = RootValue()["PipelineOrder"];
		if (!pipelineOrder.Exists())
			pipelineOrder.Set(YAML::Node(YAML::NodeType::Sequence));

		IncludedPipelines.clear();
		IncludedPipelines.resize(pipelineOrder.Size());

		for (size_t i = 0; i < pipelineOrder.Size(); ++i)
			IncludedPipelines[i] = pipelineOrder[i].As<uint64_t>();

		AddNewPipelines();
	}
}