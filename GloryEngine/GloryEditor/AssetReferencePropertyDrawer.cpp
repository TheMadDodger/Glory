#include "AssetReferencePropertyDrawer.h"
#include "AssetPickerPopup.h"
#include <imgui.h>

namespace Glory::Editor
{
	bool AssetReferencePropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		ImGui::TextUnformatted(label.c_str());
		ImGui::SameLine();

		Resource** pResourceMember = (Resource**)data;

		Resource* pValue = dynamic_cast<Resource*>(*pResourceMember);

		std::string assetName = "";
		if (*pResourceMember == nullptr || pValue->Name().data() == nullptr) assetName = "Noone";
		else
		{
			assetName = (*pResourceMember)->Name();
		}

		if (ImGui::Button(assetName.c_str()))
		{
			AssetPickerPopup::Open(typeHash, pResourceMember);
		}

		return true;
	}

	bool AssetReferencePropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const
	{
		// TODO
		return false;
	}
}