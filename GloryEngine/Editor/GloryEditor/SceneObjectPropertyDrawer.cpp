#include "SceneObjectPropertyDrawer.h"
#include "ObjectPicker.h"

namespace Glory::Editor
{
	bool SceneObjectPropertyDrawer::OnGUI(const std::string& label, SceneObjectRef* data, uint32_t flags) const
	{
		return ObjectPicker::ObjectDropdown(label, data);
	}
}
