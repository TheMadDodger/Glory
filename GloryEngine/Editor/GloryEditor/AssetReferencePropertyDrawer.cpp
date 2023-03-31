#include "AssetReferencePropertyDrawer.h"
#include "AssetPicker.h"
#include "AssetReference.h"
#include "ValueChangeAction.h"
#include <imgui.h>
#include <AssetDatabase.h>

namespace Glory::Editor
{
	bool AssetReferencePropertyDrawer::Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		AssetReferenceBase* pReferenceMember = (AssetReferenceBase*)data;
		AssetReferenceBase* oldValue = pReferenceMember->CreateCopy();
		bool change = AssetPicker::ResourceDropdown(label, pReferenceMember->TypeHash(), pReferenceMember->AssetUUIDMember());
		if (change)
		{
			ValueChangeAction* pAction = new ValueChangeAction(PropertyDrawer::GetRootTypeData(), PropertyDrawer::GetCurrentPropertyPath());
			pAction->SetOldValue(oldValue);
			pAction->SetNewValue(pReferenceMember);
			Undo::AddAction(pAction);
		}
		delete oldValue;
		return change;
	}

	bool AssetReferencePropertyDrawer::Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const
	{
		UUID uuid = node.as<uint64_t>();
		UUID oldUuid = uuid;
		if (AssetPicker::ResourceDropdown(label, typeHash, &uuid))
		{
			node = (uint64_t)uuid;
			Undo::AddAction(new NodeValueChangeAction(PropertyDrawer::GetCurrentPropertyPath(), YAML::Node((uint64_t)oldUuid), YAML::Node((uint64_t)uuid)));
			return true;
		}

		return false;
	}
}