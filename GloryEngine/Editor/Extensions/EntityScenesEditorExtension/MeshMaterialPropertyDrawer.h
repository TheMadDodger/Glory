#pragma once
#include <PropertyDrawer.h>
#include <Components.h>

namespace Glory::Editor
{
	class MeshMaterialPropertyDrawer : public SimplePropertyDrawerTemplate<MeshMaterial>
	{
	protected:
		virtual bool OnGUI(const std::string& label, MeshMaterial* data, uint32_t flags) const override;
	};
}
