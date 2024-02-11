#pragma once
#include "PropertyDrawer.h"
#include "SerializedTypes.h"

namespace Glory::Editor
{
	class AssetReferencePropertyDrawer : public PropertyDrawer
	{
	public:
		AssetReferencePropertyDrawer() : PropertyDrawer(SerializedType::ST_Asset) {}

	protected:
		virtual bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const override;
		virtual bool Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const;
		virtual bool Draw(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags) const;
	};
}
