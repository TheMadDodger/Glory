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

	private:
		bool DrawFields(void* data, const TypeData* pStructTypeData, uint32_t flags) const;
	};
}
