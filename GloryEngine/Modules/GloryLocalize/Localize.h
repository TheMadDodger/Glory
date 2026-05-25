#pragma once
#include "StringTable.h"

#include <AssetReference.h>

#include <Reflection.h>

namespace Glory
{
	/** @brief String table loader component */
	struct StringTableLoader
	{
		StringTableLoader() : m_StringTable(0ull), m_KeepLoaded(false) {}

		REFLECTABLE(StringTableLoader,
			(ResourceReference<StringTable>)(m_StringTable),
			(bool)(m_KeepLoaded)
		);
	};
	
	/** @brief Text localizer component */
	struct Localize
	{
		REFLECTABLE(Localize,
			(std::string) (m_Term)
		);
	};
}
