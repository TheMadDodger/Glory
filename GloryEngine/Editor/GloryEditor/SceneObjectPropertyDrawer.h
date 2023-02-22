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

    /* Redirects property drawing of type ST_Object to the SceneObjectPropertyDrawer above */
    class SceneObjectRedirectPropertyDrawer : public PropertyDrawer
    {
    public:
        SceneObjectRedirectPropertyDrawer() : PropertyDrawer(ST_Object) {}

    protected:
        bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const override;
        bool Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const override;
    };
}
