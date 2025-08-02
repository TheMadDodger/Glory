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

	FileData::FileData(const std::vector<char>& data)
	{
		APPEND_TYPE(FileData);
		m_Data.resize(data.size());
		std::memcpy(m_Data.data(), data.data(), data.size());
	}

	FileData::FileData(std::string_view data): m_Data(data.begin(), data.end())
	{
		APPEND_TYPE(FileData);
		m_Data.push_back('\0');
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

	std::vector<char>::iterator FileData::Begin()
	{
		return m_Data.begin();
	}

	const std::vector<char>::const_iterator FileData::Begin() const
	{
		return m_Data.cbegin();
	}

	std::vector<char>::iterator FileData::End()
	{
		return m_Data.end();
	}

	const std::vector<char>::const_iterator FileData::End() const
	{
		return m_Data.cend();
	}

	void FileData::Serialize(BinaryStream& container) const
	{
		container.Write(m_Data.size()).
			Write(m_Data.data(), m_Data.size()).Write(m_MetaData);
	}

	void FileData::Deserialize(BinaryStream& container)
	{
		size_t size;
		container.Read(size);
		m_Data.resize(size);
		container.Read(m_Data.data(), m_Data.size()).Read<char>(m_MetaData);
	}

	void FileData::SetMetaData(const void* data, size_t size)
	{
		m_MetaData.clear();
		m_MetaData.resize(size);
		std::memcpy(m_MetaData.data(), data, size);
	}
}
