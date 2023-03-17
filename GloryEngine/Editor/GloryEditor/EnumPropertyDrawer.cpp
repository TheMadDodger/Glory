#include "EnumPropertyDrawer.h"
#include "EditorUI.h"
#include <imgui.h>
#include <Reflection.h>

namespace Glory::Editor
{
	bool EnumPropertyDrawer::Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		if (data == nullptr)
		{
			ImGui::Text("Error");
			return false;
		}

		const GloryReflect::TypeData* pEnumTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(typeHash);
		if(pPropertyDrawer) return PropertyDrawer::DrawProperty(label, pEnumTypeData, data, flags);

		GloryReflect::EnumType *pEnumType = GloryReflect::Reflect::GetEnumType(typeHash);
		if (!pEnumType)
		{
			ImGui::TextColored({1,0,0,1}, label.c_str());
			return false;
		}

		std::string value;
		if (!pEnumType->ToString(data, value)) value = "none";

		uint32_t* currentValue = (uint32_t*)data;
		return EditorUI::InputEnum(label, typeHash, currentValue);
	}
}