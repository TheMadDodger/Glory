#pragma once
#include "GloryEditor.h"

#include <Resource.h>
#include <functional>
#include <SceneObjectRef.h>
#include <Entity.h>

namespace Glory::Editor
{
	class ObjectPicker
	{
	public:
		static GLORY_EDITOR_API bool ObjectDropdown(const std::string& label, SceneObjectRef* value, const float borderPadding = 0.0f);
		static GLORY_EDITOR_API bool ObjectDropdown(const std::string& label, UUID* sceneValue, UUID* objectValue, const float borderPadding = 0.0f);

	private:
		ObjectPicker();
		virtual ~ObjectPicker();

		static void RefreshFilter();
		static bool DrawPopup(UUID* sceneValue, UUID* objectValue);

	private:
		static char m_FilterBuffer[200];
		static std::string m_Filter;
		static std::vector<Entity> m_FilteredObjects;
	};
}