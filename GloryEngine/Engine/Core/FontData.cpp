#include "FontData.h"

namespace Glory
{
	FontData::FontData()
	{
		APPEND_TYPE(FontData);
	}

	FontData::FontData(FileData* pFileData): FileData(pFileData)
	{
		APPEND_TYPE(FontData);
	}

	void FontData::Serialize(BinaryStream& container) const
	{
		
	}

	void FontData::Deserialize(BinaryStream& container)
	{
		
	}
}
