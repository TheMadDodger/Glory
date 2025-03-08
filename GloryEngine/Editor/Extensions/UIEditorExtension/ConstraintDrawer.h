#pragma once
#include <PropertyDrawer.h>

#include <UIComponents.h>

namespace Glory::Editor
{
	bool DrawConstraintProperty(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags);

	template<typename T>
	class ConstraintDrawer : public PropertyDrawer
	{
	public:
		ConstraintDrawer(): PropertyDrawer(ResourceTypes::GetHash<T>()) {}

	protected:
		virtual bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const override { return false; };
		virtual bool Draw(const std::string& label, YAML::Node& node, uint32_t typeHash, uint32_t flags) const override { return false; };
		virtual bool Draw(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags) const override
		{
			return DrawConstraintProperty(file, path, typeHash, flags);
		};
	};
}
