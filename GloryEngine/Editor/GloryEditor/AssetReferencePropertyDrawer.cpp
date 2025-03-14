#include "AssetReferencePropertyDrawer.h"
#include "AssetPicker.h"
#include "AssetReference.h"
#include "ValueChangeAction.h"
#include "PropertyFlags.h"

#include <imgui.h>

namespace Glory::Editor
{
	bool AssetReferencePropertyDrawer::Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)data;
		AssetReferenceBase oldValue{ pReferenceMember->AssetUUID() };
		bool change = AssetPicker::ResourceDropdown(label, typeHash ? typeHash : pReferenceMember->TypeHash(), pReferenceMember->AssetUUIDMember());
		if (change)
		{
			ValueChangeAction* pAction = new ValueChangeAction(PropertyDrawer::GetRootTypeData(), PropertyDrawer::GetCurrentPropertyPath());
			pAction->SetOldValue(&oldValue);
			pAction->SetNewValue(pReferenceMember);
			Undo::AddAction(pAction);
		}
		return change;
	}

	bool AssetReferencePropertyDrawer::Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const
	{
		UUID uuid = node.as<uint64_t>(0);
		const UUID oldUuid = uuid;
		if (AssetPicker::ResourceDropdown(label, typeHash, &uuid))
		{
			node = (uint64_t)uuid;
			Undo::AddAction(new NodeValueChangeAction(PropertyDrawer::GetCurrentPropertyPath(), YAML::Node((uint64_t)oldUuid), YAML::Node((uint64_t)uuid)));
			return true;
		}

		return false;
	}

	bool AssetReferencePropertyDrawer::Draw(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags) const
	{
		UUID uuid = file[path].As<uint64_t>(0);
		const UUID oldUuid = uuid;

		std::string label = path.filename().string().data();
		if (label == "Value")
			label = path.parent_path().filename().string();

		if (AssetPicker::ResourceDropdown(label, typeHash, &uuid))
		{
			file[path].Set(uint64_t(uuid));
			Undo::ApplyYAMLEdit(file, path, uint64_t(oldUuid), uint64_t(uuid));
			return true;
		}

		return false;
	}
}