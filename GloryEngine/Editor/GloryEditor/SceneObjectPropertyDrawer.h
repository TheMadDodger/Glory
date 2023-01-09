#pragma once
#include "PropertyDrawer.h"
#include <SceneObjectRef.h>

namespace Glory::Editor
{
    class SceneObjectPropertyDrawer : public PropertyDrawerTemplate<SceneObjectRef>
    {
	protected:
		virtual bool OnGUI(const std::string& label, SceneObjectRef* data, uint32_t flags) const override;
    };
}
