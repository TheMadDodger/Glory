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

#include <DND.h>
#include <Undo.h>

#include <PipelineData.h>

#include <imgui.h>
#include <IconsFontAwesome6.h>
#include <BinaryStream.h>
#include <RendererModule.h>

namespace Glory::Editor
{
	std::vector<UUID> PipelineOrder;

	DND ReorderDragAndDrop{ { ResourceTypes::GetHash<PipelineData>() } };

	RenderSettings::PipelineReorderAction::PipelineReorderAction(RenderSettings* pRenderSettings, UUID movedPipeline, size_t oldIndex, size_t newIndex):
		m_pRenderSettings(pRenderSettings), m_MovedPipeline(movedPipeline), m_OldIndex(oldIndex), m_NewIndex(newIndex)
	{
	}

	void RenderSettings::PipelineReorderAction::OnUndo(const ActionRecord&)
	{
		PipelineOrder.erase(PipelineOrder.begin() + m_NewIndex);
		if (m_OldIndex >= PipelineOrder.size())
			PipelineOrder.emplace_back(m_MovedPipeline);
		else
			PipelineOrder.insert(PipelineOrder.begin() + m_OldIndex, m_MovedPipeline);

		auto pipelineOrder = m_pRenderSettings->RootValue()["PipelineOrder"];
		for (size_t i = 0; i < PipelineOrder.size(); ++i)
			pipelineOrder[i].Set(uint64_t(PipelineOrder[i]));

		m_pRenderSettings->SendToRenderer();
	}

	void RenderSettings::PipelineReorderAction::OnRedo(const ActionRecord&)
	{
		PipelineOrder.erase(PipelineOrder.begin() + m_OldIndex);
		if (m_NewIndex >= PipelineOrder.size())
			PipelineOrder.emplace_back(m_MovedPipeline);
		else
			PipelineOrder.insert(PipelineOrder.begin() + m_NewIndex, m_MovedPipeline);

		auto pipelineOrder = m_pRenderSettings->RootValue()["PipelineOrder"];
		for (size_t i = 0; i < PipelineOrder.size(); ++i)
			pipelineOrder[i].Set(uint64_t(PipelineOrder[i]));

		m_pRenderSettings->SendToRenderer();
	}

	bool RenderSettings::OnGui()
	{
		ReorderDragAndDrop.m_DNDFlags = ImGuiDragDropFlags_SourceAllowNullID | ImGuiDragDropFlags_AcceptNoDrawDefaultRect;

		static const uint32_t pipelineHash = ResourceTypes::GetHash<PipelineData>();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorAssetManager& assetManager = EditorApplication::GetInstance()->GetAssetManager();
		ResourceTypes& resourceTypes = pEngine->GetResourceTypes();
		const ResourceType* pPipelineType = resourceTypes.GetResourceType(pipelineHash);

		const ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

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

			static std::pair<UUID, size_t> toMovePipeline{0ull, 0ull};
			while (clipper.Step())
			{
				for (size_t i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
				{
					const UUID uuid = pipelineOrder[i].As<uint64_t>();
					//const size_t searchResultIndex = m_SearchResultCache.size() - (m_SearchResultCache.end() - it);

					//const size_t index = m_SearchResultIndexCache.empty()
						//? searchResultIndex : m_SearchResultIndexCache[searchResultIndex];

					ResourceMeta meta;
					EditorAssetDatabase::GetAssetMetadata(uuid, meta);
					const std::string name = EditorAssetDatabase::GetAssetName(uuid);

					AssetLocation location;
					EditorAssetDatabase::GetAssetLocation(uuid, location);

					ImGui::PushID(uuid);
					ImGui::TableNextRow();

					ImGui::TableNextColumn();

					if (ImGui::Selectable("##selectable", false, selectableFlags, ImVec2(0, rowHeight)))
					{
						Resource* pResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(uuid);
						if (!pResource) pResource = EditorApplication::GetInstance()->GetPipelineManager().GetPipelineData(uuid);
						Selection::SetActiveObject(pResource);
					}

					AssetPayload payload{ uuid };
					DND::DragAndDropSource(pPipelineType->Name(), &payload, sizeof(AssetPayload), [&]() {
						ImGui::Text(name.data());
					}, ImGuiDragDropFlags_SourceAllowNullID | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

					ReorderDragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
						const AssetPayload payload = *(const AssetPayload*)pPayload->Data;
						toMovePipeline.first = payload.ResourceID;
						toMovePipeline.second = i;
					},
					[&]() {
						auto table = ImGui::GetCurrentTable();
						ImGui::GetCurrentWindow()->DrawList->AddLine(
							ImVec2(table->BorderX1, table->RowPosY1),
							ImVec2(table->BorderX2, table->RowPosY1),
							ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_SeparatorHovered]),
							3.0);
					}
					);

					ImGui::SameLine();
					ImGui::Text("%i", i);

					ImGui::TableNextColumn();
					ImGui::Text("%s", std::to_string(uuid).data());

					ImGui::TableNextColumn();
					ImGui::Text("%s", name.data());
					
					ImGui::TableNextColumn();
					ImGui::Text("%s", location.Path.data());

					ImGui::PopID();
				}
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Selectable("##reorderend", false, selectableFlags, ImVec2(0, 2.0f));
			ReorderDragAndDrop.HandleDragAndDropTarget([&](uint32_t dndHash, const ImGuiPayload* pPayload) {
				const AssetPayload payload = *(const AssetPayload*)pPayload->Data;
				toMovePipeline.first = payload.ResourceID;
				toMovePipeline.second = PipelineOrder.size();
			},
			[&]() {
				auto table = ImGui::GetCurrentTable();
				ImGui::GetCurrentWindow()->DrawList->AddLine(
					ImVec2(table->BorderX1, table->RowPosY1),
					ImVec2(table->BorderX2, table->RowPosY1),
					ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_SeparatorHovered]),
					3.0);
			}
			);

			if (toMovePipeline.first)
			{
				auto iter = std::find(PipelineOrder.begin(), PipelineOrder.end(), toMovePipeline.first);
				const size_t index = iter - PipelineOrder.begin();
				const size_t insertIndex = toMovePipeline.second > index ? toMovePipeline.second - 1 : toMovePipeline.second;
				PipelineOrder.erase(iter);
				if (insertIndex >= PipelineOrder.size())
					PipelineOrder.emplace_back(toMovePipeline.first);
				else
					PipelineOrder.insert(PipelineOrder.begin() + insertIndex, toMovePipeline.first);

				for (size_t i = 0; i < PipelineOrder.size(); ++i)
					pipelineOrder[i].Set(uint64_t(PipelineOrder[i]));

				Undo::StartRecord("Reorder Pipelines");
				Undo::AddAction(new PipelineReorderAction(this, toMovePipeline.first, index, insertIndex));
				Undo::StopRecord();

				toMovePipeline.first = 0ull;
				toMovePipeline.second = 0ull;

				change = true;

				SendToRenderer();
			}

			ImGui::EndTable();
			ImGui::EndChild();
		}

		ImGui::EndChild();

		return change;
	}

	void RenderSettings::OnOpen()
	{
		AddNewPipelines();
	}

	void RenderSettings::OnSettingsLoaded()
	{
		VerifySettings();
	}

	void RenderSettings::OnCompile(const std::filesystem::path& path)
	{
		std::filesystem::path finalPath = path;
		finalPath.replace_filename("Renderer.dat");
		BinaryFileStream file{ finalPath };
		BinaryStream* stream = &file;
		stream->Write(CoreVersion);

		auto pipelineOrder = RootValue()["PipelineOrder"];
		const size_t count = pipelineOrder.Size();
		stream->Write(count);
		for (size_t i = 0; i < count; ++i)
		{
			const UUID pipelineID = pipelineOrder[i].As<uint64_t>();
			stream->Write(pipelineID);
		}
	}

	void RenderSettings::OnStartPlay_Impl()
	{
		SendToRenderer();
	}

	void RenderSettings::SendToRenderer()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		if (!pRenderer) return;

		std::vector<UUID> pipelineOrder(PipelineOrder);
		pRenderer->SetPipelineOrder(std::move(pipelineOrder));
	}

	void RenderSettings::AddNewPipelines()
	{
		auto pipelineOrder = RootValue()["PipelineOrder"];

		static const uint32_t pipelineHash = ResourceTypes::GetHash<PipelineData>();

		std::vector<UUID> allPipelines;
		EditorAssetDatabase::GetAllAssetsOfType(pipelineHash, allPipelines);
		for (auto pipelineID : allPipelines)
		{
			auto iter = std::find(PipelineOrder.begin(), PipelineOrder.end(), pipelineID);
			if (iter != PipelineOrder.end()) continue;
			pipelineOrder.PushBack(uint64_t(pipelineID));
			PipelineOrder.emplace_back(pipelineID);
		}

		SendToRenderer();
	}

	void RenderSettings::VerifySettings()
	{
		auto pipelineOrder = RootValue()["PipelineOrder"];
		if (!pipelineOrder.Exists())
			pipelineOrder.Set(YAML::Node(YAML::NodeType::Sequence));

		PipelineOrder.clear();
		PipelineOrder.resize(pipelineOrder.Size());

		for (size_t i = 0; i < pipelineOrder.Size(); ++i)
			PipelineOrder[i] = pipelineOrder[i].As<uint64_t>();

		AddNewPipelines();
	}
}