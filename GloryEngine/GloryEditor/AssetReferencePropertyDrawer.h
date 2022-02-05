#pragma once
#include "PropertyDrawer.h"

namespace Glory::Editor
{
	class AssetReferencePropertyDrawer : public PropertyDrawer
	{
	public:
		AssetReferencePropertyDrawer() : PropertyDrawer(SerializedType::ST_Asset) {}

	protected:
		virtual bool Draw(const std::string& label, void* data, size_t typeHash, uint32_t flags) const override;
		virtual bool Draw(const std::string& label, std::any& data, uint32_t flags) const override;
	};
}
