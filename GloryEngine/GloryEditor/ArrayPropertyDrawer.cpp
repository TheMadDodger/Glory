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

		BaseSerializedArrayProperty* serializedArrayProperty = (BaseSerializedArrayProperty*)serializedProperty;

		float width = ImGui::GetContentRegionAvailWidth();
		float inputWidth = 10.0f;
		width -= inputWidth;

		ImGui::SetNextItemWidth(width);

		std::string nodeLabel = "##" + label;
		bool node = ImGui::TreeNodeEx(nodeLabel.c_str(), 0);
		ImGui::SameLine(0, 0);
		ImGui::TextUnformatted(label.c_str());
		ImGui::SameLine(width);

		size_t size = serializedArrayProperty->ArraySize();
		int newSize = (int)size;
		std::string sizeInputLabel = nodeLabel + "_Size";
		if (ImGui::InputInt(sizeInputLabel.c_str(), &newSize, 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue) && size != (size_t)newSize)
		{
			if (newSize <= 0) newSize = 0;
			serializedArrayProperty->Resize(newSize);
		}

		if (node)
		{
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