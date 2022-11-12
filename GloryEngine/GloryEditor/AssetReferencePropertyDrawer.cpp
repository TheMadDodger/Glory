#include "AssetReferencePropertyDrawer.h"
#include "AssetPickerPopup.h"
#include "AssetReference.h"
#include <imgui.h>
#include <AssetDatabase.h>

namespace Glory::Editor
{
	bool AssetReferencePropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		ImGui::TextUnformatted(label.c_str());
		ImGui::SameLine();

		UUID* pUUIDMember = (UUID*)data;
		std::string assetName = "";
		AssetDatabase::AssetExists(*pUUIDMember);

		if (!AssetDatabase::AssetExists(*pUUIDMember)) assetName = "Noone";
		else
		{
			assetName = AssetDatabase::GetAssetName(*pUUIDMember);
		}

		std::string buttonLabel = assetName + "##" + serializedProperty->Name();

		if (ImGui::Button(buttonLabel.c_str()))
		{
			AssetPickerPopup::Open(typeHash, pUUIDMember, nullptr, true);
		}

		return true;
	}

	bool AssetReferencePropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const
	{
		// TODO
		return false;
	}

	bool AssetReferencePropertyDrawer::Draw(const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		ImGui::TextUnformatted(label.c_str());
		ImGui::SameLine();

		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)data;
		UUID uuid = pReferenceMember->AssetUUID();
		std::string assetName = "";
		bool exists = AssetDatabase::AssetExists(uuid);

		typeHash = pReferenceMember->TypeHash();

		if (!exists) assetName = "Noone";
		else
		{
			assetName = AssetDatabase::GetAssetName(uuid);
		}

		std::string buttonLabel = assetName + "##" + label;

		if (ImGui::Button(buttonLabel.c_str()))
		{
			AssetPickerPopup::Open(typeHash, pReferenceMember->AssetUUIDMember(), pReferenceMember->IsDirty(), true);
		}

		bool isDirty = *pReferenceMember->IsDirty();
		*pReferenceMember->IsDirty() = false;
		return isDirty;
	}
}