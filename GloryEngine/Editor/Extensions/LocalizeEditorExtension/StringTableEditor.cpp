#include "StringTableEditor.h"

#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>

#include <Engine.h>
#include <WindowModule.h>

#include <StringUtils.h>

#include <IconsFontAwesome6.h>
#include <StringsOverrideTable.h>

namespace Glory::Editor
{
	const size_t SearchBufferSize = 256;
	char SearchBuffer[SearchBufferSize] = "\0";
	bool ForceFilter = true;

	StringTableEditor::StringTableEditor() : EditorWindowTemplate("String Table Editor", 600.0f, 600.0f), m_TableID(0)
	{
	}

	StringTableEditor::~StringTableEditor()
	{
	}

	void StringTableEditor::SetTable(UUID tableID)
	{
		m_TableID = tableID;
	}

	void FolderGUI(Utils::NodeValueRef node, float rowHeight)
	{
		ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
		std::vector<std::string> stringKeys;

		for (auto it = node.Begin(); it != node.End(); ++it)
		{
			std::string key = *it;
			auto item = node[key];
			if (!item.IsMap())
			{
				stringKeys.push_back(std::move(key));
				continue;
			}

			ImGui::PushID(key.data());
			ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

			bool treeOpen = false;

			ImGui::TableNextColumn();
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap |
				ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

			ImGui::TreePush(key.data());
			const ImGuiID id = ImGui::GetID(key.data());
			treeOpen = ImGui::TreeNodeBehaviorIsOpen(id, treeNodeFlags);

			if (ImGui::Selectable("##selectable", false, selectableFlags, ImVec2(0, rowHeight)))
				ImGui::TreeNodeSetOpen(id, !treeOpen);
			ImGui::SameLine();
			ImGui::Text("%s %s", treeOpen ? ICON_FA_FOLDER : ICON_FA_FOLDER_CLOSED, key.data());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted("Folder");

			if (treeOpen)
				FolderGUI(item, rowHeight);
			ImGui::TreePop();
			ImGui::PopID();
		}

		ImGui::Indent();
		for (std::string_view key : stringKeys)
		{
			auto item = node[key];

			ImGui::PushID(key.data());
			ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

			if (ImGui::TableNextColumn())
			{
				if (ImGui::Selectable("##selectable", false, selectableFlags, ImVec2(0, rowHeight)))
				{

				}

				ImGui::SameLine();
				ImGui::Text(" %s", key.data());
			}

			if (ImGui::TableNextColumn())
			{
				ImGui::Text("%s", item.As<std::string>().data());
			}

			ImGui::PopID();
		}
		ImGui::Unindent();
	}

	void StringTableEditor::OnGUI()
	{
		if (!m_TableID)
		{
			ImGui::TextUnformatted("No table opened");
			return;
		}

		EditableResource* pResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(m_TableID);
		ResourceMeta meta;
		if (!pResource || !EditorAssetDatabase::GetAssetMetadata(m_TableID, meta))
		{
			ImGui::TextUnformatted("Invalid table opened");
			return;
		}

		YAMLResourceBase* pYAMLResource = static_cast<YAMLResourceBase*>(pResource);
		Utils::YAMLFileRef& file = **pYAMLResource;

		auto rootNode = file.RootNodeRef().ValueRef();
		if (!rootNode.Exists() || !rootNode.IsMap()) rootNode.SetMap();
		auto rootFolder = meta.Hash() == ResourceTypes::GetHash<StringsOverrideTable>() ? rootNode["Overrides"] : rootNode;
		if (!rootFolder.Exists() || !rootFolder.IsMap()) rootFolder.SetMap();

		static const ImGuiTableFlags flags =
			ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg
			| ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;

		const float rowHeight = 32.0f;

		const float regionWidth = ImGui::GetWindowContentRegionMax().x;
		bool needsFilter = EditorUI::SearchBar(regionWidth, SearchBuffer, SearchBufferSize) || ForceFilter;
		ForceFilter = false;

		if (needsFilter)
		{

		}

		if (!ImGui::BeginChild("TextChild") || !ImGui::BeginTable("TextTable", 2, flags))
		{
			ImGui::EndChild();
			return;
		}

		ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.3f, 0);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 0.7f, 1);

		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableHeadersRow();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorAssetManager& assetManager = EditorApplication::GetInstance()->GetAssetManager();
		ResourceTypes& resourceTypes = pEngine->GetResourceTypes();

		ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

		bool treeOpen = false;
		if (ImGui::TableNextColumn())
		{
			const ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

			ImGui::TreePush("root");
			const ImGuiID id = ImGui::GetID("root");
			treeOpen = ImGui::TreeNodeBehaviorIsOpen(id, treeNodeFlags);

			if (ImGui::Selectable("##selectable", false, selectableFlags, ImVec2(0, rowHeight)))
				ImGui::TreeNodeSetOpen(id, !treeOpen);

			ImGui::SameLine();
			ImGui::Text("%s Root", treeOpen ? ICON_FA_FOLDER : ICON_FA_FOLDER_CLOSED);
		}

		if (ImGui::TableNextColumn()) {}

		if (treeOpen)
			FolderGUI(rootFolder, rowHeight);
		ImGui::TreePop();

		ImGui::EndTable();
		ImGui::EndChild();
	}

	void StringTableEditor::Update()
	{
	}

	void StringTableEditor::Draw()
	{
	}
}
