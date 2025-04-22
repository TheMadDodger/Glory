#pragma once
#include "StringTable.h"

#include <AssetReference.h>

#include <Reflection.h>

namespace Glory
{
	struct StringTableRef
	{
		StringTableRef() : m_STReference(0) {}
		StringTableRef(UUID uuid) : m_STReference(uuid) {}

		bool operator==(const StringTableRef&& other)
		{
			return other.m_STReference.AssetUUID() == other.m_STReference.AssetUUID();
		}

		bool operator==(StringTableRef& other)
		{
			return other.m_STReference.AssetUUID() == other.m_STReference.AssetUUID();
		}

		REFLECTABLE(StringTableRef,
			(AssetReference<StringTable>) (m_STReference)
		)
	};

	/** @brief String table loader component */
	struct StringTableLoader
	{
		REFLECTABLE(StringTableLoader,
			(std::vector<StringTableRef>)(m_StringTables)
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
