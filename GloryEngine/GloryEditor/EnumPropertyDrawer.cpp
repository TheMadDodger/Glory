#include "EnumPropertyDrawer.h"
#include <imgui.h>
#include <Reflection.h>

namespace Glory::Editor
{
	bool EnumPropertyDrawer::Draw(const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		if (data == nullptr)
		{
			ImGui::Text("Error");
			return false;
		}

		bool change = false;

		const GloryReflect::TypeData* pEnumTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(typeHash);
		if(pPropertyDrawer) return PropertyDrawer::DrawProperty(label, pEnumTypeData, data, flags, label);

		GloryReflect::EnumType *pEnumType = GloryReflect::Reflect::GetEnumType(typeHash);
		if (!pEnumType)
		{
			ImGui::TextColored({1,0,0,1}, label.c_str());
			return false;
		}

		std::string value;
		if (!pEnumType->ToString(data, value)) value = "none";

		size_t* currentValue = (size_t*)data;
		if (ImGui::BeginCombo(label.c_str(), value.c_str()))
		{
			for (size_t i = 0; i < pEnumType->NumValues(); i++)
			{
				const std::string& name = pEnumType->GetName(i);
				size_t outValue = 0;
				pEnumType->FromString(name, (void*)&outValue);

				if (ImGui::Selectable(name.c_str(), outValue == *currentValue))
				{
					*currentValue = outValue;
					change = true;
				}
			}
			ImGui::EndCombo();
		}

		return change;
	}
}