#include "TextDatabase.h"

namespace Glory
{
	TextDatabase::TextDatabase()
	{
	}

	TextDatabase::~TextDatabase()
	{
	}

	void TextDatabase::References(Engine*, std::vector<UUID>&) const
	{
	}

	void TextDatabase::Serialize(BinaryStream& container) const
	{
		/* Should package a file per language, so this code should never run */
	}

	void TextDatabase::Deserialize(BinaryStream& container)
	{
	}
}
