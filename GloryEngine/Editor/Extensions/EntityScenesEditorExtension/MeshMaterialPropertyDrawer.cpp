#include <AssetPicker.h>
#include <PropertyDrawer.h>
#include <Components.h>

namespace Glory::Editor
{
	bool SimplePropertyDrawerTemplate<MeshMaterial>::OnGUI(const std::string& label, MeshMaterial* data, uint32_t flags) const
	{
		AssetReference<MaterialData>* pReferenceMember = &data->m_MaterialReference;
		return AssetPicker::ResourceDropdown(label, pReferenceMember->TypeHash(), pReferenceMember->AssetUUIDMember());
	}
}
