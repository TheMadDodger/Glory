#include "StructPropertyDrawer.h"
#include "EditorUI.h"

#include <imgui.h>
#include <Reflection.h>

namespace Glory::Editor
{
	bool StructPropertyDrawer::Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		if (data == nullptr)
		{
			ImGui::Text("Error");
			return false;
		}

		bool change = false;

		const TypeData* pStructTypeData = Reflect::GetTyeData(typeHash);
		PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(typeHash);
		if (pPropertyDrawer) return PropertyDrawer::DrawProperty(label, pStructTypeData, data, flags);

		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		ImGui::PushID(label.c_str());

		if (label.empty())
		{
			change |= DrawFields(data, pStructTypeData, flags);
			ImGui::PopID();
			return change;
		}

		bool mainToggle = false;
		if (pStructTypeData->FieldCount())
		{
			static uint32_t booleanType = ResourceTypes::GetHash<bool>();
			const FieldData* pFieldData = pStructTypeData->GetFieldData(0);
			mainToggle = strcmp(pFieldData->Name(), "m_Enable") == 0 && pFieldData->ArrayElementType() == booleanType;
		}

		bool headerOpen = false;
		if (mainToggle)
		{
			node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_AllowItemOverlap;
			headerOpen = ImGui::TreeNodeEx("node", node_flags, EditorUI::MakeCleanName(label).data());
			const FieldData* pFieldData = pStructTypeData->GetFieldData(0);
			const size_t offset = pFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);
			bool enabled = false;
			ImGui::SameLine();
			const float cursorX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 25.0f;
			ImGui::SetCursorPosX(cursorX);
			EditorUI::PushFlag(EditorUI::Flag::NoLabel);
			change |= PropertyDrawer::DrawProperty(pFieldData, pAddress, flags);
			EditorUI::PopFlag();
			pFieldData->Get(pAddress, &enabled);
			ImGui::BeginDisabled(!enabled);
		}
		else
		{
			headerOpen = ImGui::TreeNodeEx("node", node_flags, EditorUI::MakeCleanName(label).data());
		}

		if(headerOpen)
		{
			ImGui::Indent(5.0f);
			change |= DrawFields(data, pStructTypeData, flags, mainToggle ? 1 : 0);
			if (!mainToggle) ImGui::TreePop();
			ImGui::Unindent(5.0f);
		}
		if (mainToggle)
		{
			ImGui::EndDisabled();
		}
		ImGui::PopID();
		return change;
	}

	bool StructPropertyDrawer::Draw(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags) const
	{
		auto structData = file[path];

		const TypeData* pStructTypeData = Reflect::GetTyeData(typeHash);
		bool change = false;
		for (size_t i = 0; i < pStructTypeData->FieldCount(); ++i)
		{
			const FieldData* pFieldData = pStructTypeData->GetFieldData(i);
			auto field = structData[pFieldData->Name()];
			const uint32_t fieldFlags = uint32_t(Reflect::GetFieldFlags(pFieldData));
			change |= PropertyDrawer::DrawProperty(file, field.Path(), pFieldData->Type(), pFieldData->ArrayElementType(), fieldFlags);
		}
		return change;
	}

	bool StructPropertyDrawer::DrawFields(void* data, const TypeData* pStructTypeData, uint32_t flags, size_t start) const
	{
		bool change = false;
		for (size_t i = start; i < pStructTypeData->FieldCount(); ++i)
		{
			const FieldData* pFieldData = pStructTypeData->GetFieldData(i);
			const size_t offset = pFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);
			const uint32_t fieldFlags = uint32_t(Reflect::GetFieldFlags(pFieldData));
			change |= PropertyDrawer::DrawProperty(pFieldData, pAddress, fieldFlags);
		}
		return change;
	}
}