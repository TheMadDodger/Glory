#pragma once
#include "PropertyDrawer.h"
#include "SerializedTypes.h"

namespace Glory::Editor
{
	class ArrayPropertyDrawer : public PropertyDrawer
	{
	public:
		ArrayPropertyDrawer() : PropertyDrawer(SerializedType::ST_Array) {}

	protected:
		virtual bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const override;
		virtual bool Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t elementTypeHash, uint32_t flags) const override;
	};
}
