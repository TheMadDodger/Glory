#include "StringTableEditor.h"

#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <EditorAssetDatabase.h>
#include <Undo.h>
#include <ObjectMenuCallbacks.h>

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
	bool TextArea = false;

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

	bool CreatingNewItem = false;
	bool CreatingNewFolder = false;
	std::filesystem::path CreatingKeyPath;
	char KeyBuffer[256] = "\0";
	char ValueBuffer[1024] = "\0";
	bool KeyFinished = false;
	std::filesystem::path ToRemovePath;
	std::filesystem::path EditingItemPath;
	std::filesystem::path EditingKeyPath;

	std::filesystem::path MoveFrom;
	std::filesystem::path MoveTo;

	void FolderRightClickMenu(Utils::NodeValueRef item, bool hasRemoveAndRename)
	{
		if (ImGui::BeginPopup("FolderRightClick"))
		{
			if (hasRemoveAndRename && ImGui::MenuItem("Remove", "", false))
			{
				ImGui::CloseCurrentPopup();
				ToRemovePath = item.Path();
			}
			if (ImGui::MenuItem("Add Key", "", false))
			{
				ImGui::CloseCurrentPopup();
				EditingKeyPath = "";
				EditingItemPath = "";

				CreatingKeyPath = item.Path();
				CreatingNewItem = true;
				CreatingNewFolder = false;
				std::strcpy(KeyBuffer, "New Key");
				ValueBuffer[0] = '\0';
			}
			if (ImGui::MenuItem("Add Sub Folder", "", false))
			{
				ImGui::CloseCurrentPopup();
				EditingKeyPath = "";
				EditingItemPath = "";

				CreatingKeyPath = item.Path();
				CreatingNewItem = true;
				CreatingNewFolder = true;
				std::strcpy(KeyBuffer, "New Folder");
				ValueBuffer[0] = '\0';
			}
			if (hasRemoveAndRename && ImGui::MenuItem("Rename", "", false))
			{
				ImGui::CloseCurrentPopup();
				CreatingNewItem = false;
				CreatingNewFolder = false;
				CreatingKeyPath = "";
				EditingItemPath = "";

				EditingKeyPath = item.Path();
				const std::string key = item.Path().filename().string();
				std::strcpy(KeyBuffer, key.data());
			}
			ImGui::EndPopup();
		}
	}

	void ItemRightClickMenu(Utils::NodeValueRef item)
	{
		if (ImGui::BeginPopup("ItemRightClick"))
		{
			if (ImGui::MenuItem("Remove", "", false))
			{
				ImGui::CloseCurrentPopup();
				ToRemovePath = item.Path();
			}
			if (ImGui::MenuItem("Edit", "", false))
			{
				ImGui::CloseCurrentPopup();
				CreatingNewItem = false;
				CreatingNewFolder = false;
				CreatingKeyPath = "";
				EditingKeyPath = "";

				EditingItemPath = item.Path();
				const std::string value = item.As<std::string>();
				std::strcpy(ValueBuffer, value.data());
			}
			if (ImGui::MenuItem("Rename", "", false))
			{
				ImGui::CloseCurrentPopup();
				CreatingNewItem = false;
				CreatingNewFolder = false;
				CreatingKeyPath = "";
				EditingItemPath = "";

				EditingKeyPath = item.Path();
				const std::string key = item.Path().filename().string();
				std::strcpy(KeyBuffer, key.data());
			}
			ImGui::EndPopup();
		}
	}

	void ValueEdit(bool& change, bool& confirm, float rowHeight)
	{
		constexpr float buttonWidth = 32.0f;
		const float cellPadding = ImGui::GetStyle().CellPadding.y*2.0f;

		if (!TextArea)
		{
			change = ImGui::InputTextEx("##valueInput", nullptr, ValueBuffer, 256,
				{ ImGui::GetContentRegionAvail().x - buttonWidth - cellPadding, 0.0f },
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput);
			confirm = change;
		}
		else
		{
			change = ImGui::InputTextMultiline("##valueInput", ValueBuffer, 1024,
				{ ImGui::GetContentRegionAvail().x - buttonWidth - cellPadding, rowHeight*5.0f - cellPadding },
				ImGuiInputTextFlags_AllowTabInput);
			confirm = ImGui::IsItemDeactivated();
		}

		ImGui::SameLine();
		ImGui::Button(TextArea ? ICON_FA_ARROW_UP : ICON_FA_ARROW_DOWN, { buttonWidth, 0.0f });
		if (ImGui::IsItemClicked())
			TextArea = !TextArea;
	}

	void StringTableEditor::FolderGUI(Utils::YAMLFileRef& file, Utils::NodeValueRef node, float rowHeight)
	{
		const ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
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

			if (ImGui::IsItemClicked(1))
				ImGui::OpenPopup("FolderRightClick");

			ImGui::SameLine();
			if (EditingKeyPath == item.Path())
			{
				ImGui::SetKeyboardFocusHere();
				const bool change = ImGui::InputTextEx("##keyInput", nullptr, KeyBuffer, 256, { 0.0f, 0.0f },
					ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput);
				if ((ImGui::IsItemDeactivated() || change))
				{
					auto newNode = node[KeyBuffer];
					MoveFrom = EditingKeyPath;
					MoveTo = newNode.Path();
				}
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) EditingKeyPath = "";
			}
			else ImGui::Text("%s %s", treeOpen ? ICON_FA_FOLDER : ICON_FA_FOLDER_CLOSED, key.data());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted("Folder");

			FolderRightClickMenu(item, true);

			if (treeOpen)
				FolderGUI(file, item, rowHeight);
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
					CreatingNewItem = false;

				if (ImGui::IsItemClicked(1))
					ImGui::OpenPopup("ItemRightClick");

				ImGui::SameLine();

				if (EditingKeyPath == item.Path())
				{
					ImGui::SetKeyboardFocusHere();
					const bool change = ImGui::InputTextEx("##keyInput", nullptr, KeyBuffer, 256, { 0.0f, 0.0f },
						ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput);
					if ((ImGui::IsItemDeactivated() || change) && !node[KeyBuffer].Exists())
					{
						auto newNode = node[KeyBuffer];
						Undo::StartRecord("Edit Value", m_TableID);
						Undo::YAMLEdit(file, newNode.Path(), YAML::Node(YAML::NodeType::Null), item.Node());
						Undo::YAMLEdit(file, EditingKeyPath, item.Node(), YAML::Node(YAML::NodeType::Null));
						Undo::StopRecord();
						EditingKeyPath = "";

						ImGui::PopID();
						continue;
					}
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) EditingKeyPath = "";
				}
				else ImGui::Text(" %s", key.data());
			}

			if (ImGui::TableNextColumn())
			{
				if (EditingItemPath == item.Path())
				{
					bool change = false;
					bool confirm = false;

					ImGui::SetKeyboardFocusHere();
					ValueEdit(change, confirm, rowHeight);

					if (confirm)
					{
						const std::string oldValue = file[EditingItemPath].As<std::string>();
						const std::string newValue = ValueBuffer;

						Undo::StartRecord("Edit Value", m_TableID);
						Undo::ApplyYAMLEdit(file, EditingItemPath, oldValue, newValue);
						Undo::StopRecord();
						EditingItemPath = "";
					}
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) EditingItemPath = "";
				}
				else ImGui::Text("%s", item.As<std::string>().data());
			}

			ItemRightClickMenu(item);

			ImGui::PopID();
		}

		NewItemGUI(file, node, rowHeight);		
		ImGui::Unindent();
	}

	void StringTableEditor::NewItemGUI(Utils::YAMLFileRef& file, Utils::NodeValueRef node, float rowHeight)
	{
		const ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

		if (CreatingNewItem && CreatingKeyPath == node.Path())
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || !m_IsFocused)
			{
				CreatingNewItem = false;
				CreatingNewFolder = false;
				KeyFinished = false;
			}

			ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight * (TextArea ? 5.0f : 1.0f));

			if (ImGui::TableNextColumn())
			{
				if (ImGui::Selectable("##selectable", false, selectableFlags, ImVec2(0, rowHeight * (TextArea ? 5.0f : 1.0f))))
				{
					CreatingNewItem = false;
					CreatingNewFolder = false;
				}

				if (ImGui::IsItemClicked())
					KeyFinished = false;

				ImGui::SameLine();
				if (!KeyFinished)
					ImGui::SetKeyboardFocusHere();
				const bool change = ImGui::InputTextEx("##keyInput", nullptr, KeyBuffer, 256, { 0.0f, 0.0f },
					ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput);
				if ((ImGui::IsItemDeactivated() || change) && !node[KeyBuffer].Exists())
					KeyFinished = true;
			}

			if (CreatingNewFolder && KeyFinished && node[KeyBuffer].Exists())
				KeyFinished = false;

			if (CreatingNewFolder && KeyFinished && !node[KeyBuffer].Exists())
			{
				auto newNode = node[KeyBuffer];
				if (!newNode.Exists())
				{
					Undo::StartRecord("New Folder", m_TableID);
					Undo::YAMLEdit(file, newNode.Path(), YAML::Node(YAML::NodeType::Null), YAML::Node(YAML::NodeType::Map));
					Undo::StopRecord();
				}

				KeyFinished = false;
				std::strcpy(KeyBuffer, "New Folder");
				ValueBuffer[0] = '\0';
			}

			if (!CreatingNewFolder && ImGui::TableNextColumn())
			{
				if (KeyFinished)
					ImGui::SetKeyboardFocusHere();

				bool change = false;
				bool confirm = false;
				
				ValueEdit(change, confirm, rowHeight);

				if (confirm)
				{
					auto newNode = node[KeyBuffer];
					if (!newNode.Exists())
					{
						YAML::Node newValue{ YAML::NodeType::Scalar };
						newValue = ValueBuffer;

						Undo::StartRecord("New Key", m_TableID);
						Undo::YAMLEdit(file, newNode.Path(), YAML::Node(YAML::NodeType::Null), newValue);
						Undo::StopRecord();
					}

					KeyFinished = false;
					std::strcpy(KeyBuffer, "New Key");
					ValueBuffer[0] = '\0';
				}
			}
			else if (ImGui::TableNextColumn())
			{
				ImGui::TextUnformatted("Folder");
			}
		}
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

		const float rowHeight = 22.0f;

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
		const std::string tableName = EditorAssetDatabase::GetAssetName(m_TableID);

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

			if (ImGui::IsItemClicked(1))
				ImGui::OpenPopup("FolderRightClick");

			FolderRightClickMenu(rootFolder, false);

			ImGui::SameLine();
			ImGui::Text("%s %s", treeOpen ? ICON_FA_FOLDER : ICON_FA_FOLDER_CLOSED, tableName.data());
		}

		if (ImGui::TableNextColumn()) {}

		if (treeOpen)
			FolderGUI(file, rootFolder, rowHeight);
		ImGui::TreePop();

		if (!ToRemovePath.empty())
		{
			Undo::StartRecord("Remove Folder", m_TableID);
			Undo::YAMLEdit(file, ToRemovePath, rootNode[ToRemovePath].Node(), YAML::Node(YAML::NodeType::Null));
			Undo::StopRecord();
			ToRemovePath.clear();
		}

		ImGui::EndTable();
		ImGui::EndChild();

		if (!MoveFrom.empty() && !MoveTo.empty())
		{
			if (!file[MoveTo].Exists())
			{
				auto oldNode = file[MoveFrom];
				auto newNode = file[MoveTo];
				Undo::StartRecord("Edit Value", m_TableID);
				Undo::YAMLEdit(file, newNode.Path(), YAML::Node(YAML::NodeType::Null), oldNode.Node());
				Undo::YAMLEdit(file, EditingKeyPath, oldNode.Node(), YAML::Node(YAML::NodeType::Null));
				Undo::StopRecord();
				EditingKeyPath.clear();
			}

			if (MoveFrom == MoveTo) EditingKeyPath.clear();

			MoveFrom.clear();
			MoveTo.clear();
		}
	}

	void StringTableEditor::Update()
	{
	}

	void StringTableEditor::Draw()
	{
	}
}
