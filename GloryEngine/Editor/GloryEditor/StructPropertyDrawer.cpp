#include "StructPropertyDrawer.h"
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

		const GloryReflect::TypeData* pStructTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(typeHash);
		if (pPropertyDrawer) return PropertyDrawer::DrawProperty(label, pStructTypeData, data, flags);

		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		ImGui::PushID(label.c_str());
		if (ImGui::TreeNodeEx("node", node_flags, label.data()))
		{
			for (size_t i = 0; i < pStructTypeData->FieldCount(); i++)
			{
				const GloryReflect::FieldData* pFieldData = pStructTypeData->GetFieldData(i);
				size_t offset = pFieldData->Offset();
				void* pAddress = (void*)((char*)(data)+offset);
				change |= PropertyDrawer::DrawProperty(pFieldData, pAddress, flags);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
		return change;
	}
}