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

		const TypeData* pEnumTypeData = Reflect::GetTyeData(typeHash);
		PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(typeHash);
		if(pPropertyDrawer) return PropertyDrawer::DrawProperty(label, pEnumTypeData, data, flags);

		EnumType *pEnumType = Reflect::GetEnumType(typeHash);
		if (!pEnumType)
		{
			ImGui::TextColored({1,0,0,1}, label.c_str());
			return false;
		}

		std::string value;
		if (!pEnumType->ToString(data, value)) value = "none";

		uint32_t* currentValue = (uint32_t*)data;
		return EditorUI::InputEnum(EditorUI::MakeCleanName(label), typeHash, currentValue);
	}

	bool EnumPropertyDrawer::Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const
	{
		const TypeData* pEnumTypeData = Reflect::GetTyeData(typeHash);
		EnumType* pEnumType = Reflect::GetEnumType(typeHash);
		if (!pEnumType)
		{
			ImGui::TextColored({ 1,0,0,1 }, label.c_str());
			return false;
		}

		std::string value = node.as<std::string>();

		uint32_t currentValue = 0;
		if (!pEnumType->FromString(value, &currentValue)) currentValue = 0;

		if (EditorUI::InputEnum(EditorUI::MakeCleanName(label), typeHash, &currentValue))
		{
			if (!pEnumType->ToString(&currentValue, value)) value = "none";
			node = value;
			return true;
		}
		return false;
	}

	bool EnumPropertyDrawer::Draw(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags) const
	{
		std::string label = path.filename().string().data();
		if (label == "Value")
			label = path.parent_path().filename().string();

		const TypeData* pEnumTypeData = Reflect::GetTyeData(typeHash);
		EnumType* pEnumType = Reflect::GetEnumType(typeHash);
		if (!pEnumType)
		{
			ImGui::TextColored({ 1,0,0,1 }, label.c_str());
			return false;
		}

		uint32_t currentValue = 0;
		std::string value;
		auto prop = file[path];
		if (!prop.Exists() && pEnumType->ToString(&currentValue, value))
			prop.Set(value);

		value = prop.As<std::string>();
		const std::string originalValue = value;

		if (!pEnumType->FromString(value, &currentValue)) currentValue = 0;

		if (EditorUI::InputEnum(EditorUI::MakeCleanName(label), typeHash, &currentValue))
		{
			if (!pEnumType->ToString(&currentValue, value)) value = "none";
			Undo::ApplyYAMLEdit(file, path, originalValue, value);
			prop.Set(value);
			return true;
		}
		return false;
	}
}