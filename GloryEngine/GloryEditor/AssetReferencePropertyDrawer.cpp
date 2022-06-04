#include "AssetReferencePropertyDrawer.h"
#include "AssetPickerPopup.h"
#include <imgui.h>

namespace Glory::Editor
{
	bool AssetReferencePropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		ImGui::TextUnformatted(label.c_str());
		ImGui::SameLine();

		// TODO: Use GUID instead of resource pointer to prevent crash, also allows assets to be loaded while scene is already open
		Resource** pResourceMember = (Resource**)data;
		std::string assetName = "";
		if (*pResourceMember == nullptr) assetName = "Noone";
		else
		{
			assetName = (*pResourceMember)->Name();
		}

		std::string buttonLabel = assetName + "##" + serializedProperty->Name();

		if (ImGui::Button(buttonLabel.c_str()))
		{
			AssetPickerPopup::Open(typeHash, pResourceMember, true);
		}

		return true;
	}

	bool AssetReferencePropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const
	{
		// TODO
		return false;
	}
}