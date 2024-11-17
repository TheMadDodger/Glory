#include "FileData.h"
#include "BinaryStream.h"

namespace Glory
{
	FileData::FileData() : m_Data(std::vector<char>())
	{
		APPEND_TYPE(FileData);
	}

	FileData::FileData(FileData* pFileData) : m_Data(pFileData->m_Data)
	{
		APPEND_TYPE(FileData);
	}
	
	FileData::FileData(std::vector<char>&& data) : m_Data(std::move(data))
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

	void FileData::Serialize(BinaryStream& container) const
	{
		container.Write(m_Data.size()).Write(m_Data.data(), m_Data.size());
	}

	void FileData::Deserialize(BinaryStream& container)
	{
		size_t size;
		container.Read(size);
		m_Data.resize(size);
		container.Read(m_Data.data(), m_Data.size());
	}
}
