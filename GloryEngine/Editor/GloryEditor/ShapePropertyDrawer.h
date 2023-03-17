#pragma once
#include "PropertyDrawer.h"
#include <ShapeProperty.h>

namespace Glory::Editor
{
    class ShapePropertyDrawer : public PropertyDrawer
    {
	public:
		ShapePropertyDrawer() : PropertyDrawer(ResourceType::GetHash<ShapeProperty>()) {}
		virtual ~ShapePropertyDrawer();

	protected:
		virtual bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const override;
		virtual bool Draw(const std::string& label, std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, uint32_t flags) const override;

		bool OnGUI(const std::string& label, ShapeProperty* data, uint32_t flags) const;
	};
}
