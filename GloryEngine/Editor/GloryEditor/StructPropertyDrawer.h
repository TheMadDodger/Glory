#pragma once
#include "PropertyDrawer.h"
#include "SerializedTypes.h"

namespace Glory::Editor
{
	class StructPropertyDrawer : public PropertyDrawer
	{
	public:
		StructPropertyDrawer() : PropertyDrawer(SerializedType::ST_Struct) {}

	protected:
		virtual bool Draw(const std::string& label, void* data, uint32_t typeHash, uint32_t flags) const override;
		virtual bool Draw(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags) const override;

	private:
		bool DrawFields(void* data, const TypeData* pStructTypeData, uint32_t flags, size_t start=0) const;
		bool DrawFields(Utils::YAMLFileRef& file, const std::filesystem::path& path, const TypeData* pStructTypeData, uint32_t flags, size_t start=0) const;
	};
}
