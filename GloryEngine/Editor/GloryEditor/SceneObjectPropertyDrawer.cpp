#include "SceneObjectPropertyDrawer.h"
#include "ObjectPicker.h"

namespace Glory::Editor
{
	bool SceneObjectPropertyDrawer::OnGUI(const std::string& label, SceneObjectRef* data, uint32_t flags) const
	{
		return ObjectPicker::ObjectDropdown(label, data);
	}

	bool SceneObjectRedirectPropertyDrawer::Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const
	{
		return PropertyDrawer::DrawProperty(label, data, ResourceType::GetHash<SceneObjectRef>(), flags);
	}

	bool SceneObjectRedirectPropertyDrawer::Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const
	{
		const uint32_t sceneObjectRefType = ResourceType::GetHash<SceneObjectRef>();
		PropertyDrawer* pDrawer = PropertyDrawer::GetPropertyDrawer(sceneObjectRefType);
		if (!pDrawer)
		{
			Debug::LogFatalError("Missing SceneObjectPropertyDrawer!");
			return false;
		}
		return pDrawer->Draw(label, node, typeHash, flags);
	}
}
