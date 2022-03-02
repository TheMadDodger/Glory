#include "ArrayPropertyDrawer.h"
#include <SerializedArrayProperty.h>
#include <imgui.h>

namespace Glory::Editor
{
	bool ArrayPropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		if (serializedProperty == nullptr)
		{
			ImGui::Text("Error");
			return false;
		}

		bool node = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
		if (node)
		{
			const BaseSerializedArrayProperty* serializedArrayProperty = (const BaseSerializedArrayProperty*)serializedProperty;

			for (size_t i = 0; i < serializedArrayProperty->ArraySize(); i++)
			{
				SerializedProperty* pChildProperty = serializedArrayProperty->GetArrayElementAt(i);
				PropertyDrawer::DrawProperty(pChildProperty);
			}

			ImGui::TreePop();
		}

		return true;
	}

	bool ArrayPropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const
	{
		// TODO
		return false;
	}
}