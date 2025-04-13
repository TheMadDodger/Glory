#include "TextFileData.h"

namespace Glory
{
	TextFileData::TextFileData() : FileData()
	{
		APPEND_TYPE(TextFileData);
	}

	TextFileData::TextFileData(FileData* pFileData) : FileData(pFileData)
	{
		APPEND_TYPE(TextFileData);
	}

	TextFileData::TextFileData(std::vector<char>&& data) : FileData(std::move(data))
	{
		APPEND_TYPE(TextFileData);
	}

	TextFileData::TextFileData(std::string_view data) : FileData(data)
	{
		APPEND_TYPE(TextFileData);
	}
}
