#pragma once
#include "SerializedTypes.h"

namespace Glory
{
	struct ScriptProperty
	{
		const char* m_Name;
		size_t m_TypeHash;
		size_t m_ElementTypeHash;
		const char* m_TypeName;
		size_t m_Size;
		SerializedType ValueType;
		size_t m_ChildrenOffset;
		size_t m_ChildrenCount;
	};
}
