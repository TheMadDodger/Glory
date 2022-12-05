#include "AssetReferencePropertyDrawer.h"
#include "AssetPicker.h"
#include "AssetReference.h"
#include <imgui.h>
#include <AssetDatabase.h>

namespace Glory::Editor
{
	bool AssetReferencePropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		UUID* pUUIDMember = (UUID*)data;
		return AssetPicker::ResourceDropdown(label, typeHash, pUUIDMember);
	}

	bool AssetReferencePropertyDrawer::Draw(const SerializedProperty* serializedProperty, const std::string& label, std::any& data, uint32_t flags) const
	{
		// TODO
		return false;
	}

	bool AssetReferencePropertyDrawer::Draw(const std::string& label, void* data, size_t typeHash, uint32_t flags) const
	{
		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)data;
		return AssetPicker::ResourceDropdown(label, pReferenceMember->TypeHash(), pReferenceMember->AssetUUIDMember());
	}

	bool AssetReferencePropertyDrawer::Draw(const std::string& label, YAML::Node& node, size_t typeHash, uint32_t flags) const
	{
		UUID uuid = node.as<uint64_t>();
		if (AssetPicker::ResourceDropdown(label, typeHash, &uuid))
		{
			node = (uint64_t)uuid;
			return true;
		}

		return false;
	}
}