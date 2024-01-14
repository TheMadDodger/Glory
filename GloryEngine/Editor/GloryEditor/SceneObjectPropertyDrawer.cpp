#include "SceneObjectPropertyDrawer.h"
#include "ObjectPicker.h"
#include "EditorApplication.h"

#include <Debug.h>

namespace Glory::Editor
{
	template<>
	bool PropertyDrawerTemplate<SceneObjectRef>::OnGUI(const std::string& label, SceneObjectRef* data, uint32_t flags) const
	{
		return ObjectPicker::ObjectDropdown(label, data);
	}

	bool SceneObjectRedirectPropertyDrawer::Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		return PropertyDrawer::DrawProperty(label, data, ResourceTypes::GetHash<SceneObjectRef>(), flags);
	}

	bool SceneObjectRedirectPropertyDrawer::Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const
	{
		const uint32_t sceneObjectRefType = ResourceTypes::GetHash<SceneObjectRef>();
		PropertyDrawer* pDrawer = PropertyDrawer::GetPropertyDrawer(sceneObjectRefType);
		if (!pDrawer)
		{
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogFatalError("Missing SceneObjectPropertyDrawer!");
			return false;
		}
		return pDrawer->Draw(label, node, typeHash, flags);
	}
}
