#include "FileData.h"

namespace Glory
{
	FileData::FileData() : m_Data(std::vector<char>())
	{
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
