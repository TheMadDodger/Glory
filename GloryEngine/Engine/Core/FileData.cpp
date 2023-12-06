#include "FileData.h"

namespace Glory
{
	FileData::FileData() : m_Data(std::vector<char>())
	{
		APPEND_TYPE(FileData);
	}

	FileData::FileData(UUID uuid, const std::string& name): Resource(uuid, name), m_Data(std::vector<char>())
	{
		APPEND_TYPE(FileData);
	}

	FileData::FileData(FileData* pFileData) : m_Data(pFileData->m_Data)
	{
		APPEND_TYPE(FileData);
	}

	FileData::~FileData()
	{
		m_Data.clear();
	}

	size_t FileData::Size()
	{
		return m_Data.size();
	}

	const char* FileData::Data()
	{
		return m_Data.data();
	}
}
