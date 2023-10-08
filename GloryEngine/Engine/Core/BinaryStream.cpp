#include "BinaryStream.h"

namespace Glory
{
	BinaryFileStream::BinaryFileStream(const std::filesystem::path& path) :
		m_File(), m_Size(0), m_Tell(0)
	{
		//m_File.open(path.string(), std::fstream::in | std::fstream::out | std::fstream::trunc);
		m_File.open(path.string(), std::fstream::out);
		m_File.seekg(std::ios_base::end);
		m_Size = m_File.tellg();
		m_File.seekg(std::ios_base::beg);
	}

	BinaryFileStream::~BinaryFileStream()
	{
		m_File.close();
	}

	void BinaryFileStream::Seek(size_t offset, Relative relative)
	{
		switch (relative)
		{
		case Glory::BinaryStream::Relative::Start:
			m_File.seekg(offset, std::ios_base::beg);
			break;
		case Glory::BinaryStream::Relative::Current:
			m_File.seekg(offset, std::ios_base::cur);
			break;
		case Glory::BinaryStream::Relative::End:
			m_File.seekg(offset, std::ios_base::end);
			break;
		default:
			break;
		}

		m_Tell = m_File.tellg();
	}

	size_t BinaryFileStream::Tell() const
	{
		return m_Tell;
	}

	size_t BinaryFileStream::Size() const
	{
		return m_Size;
	}

	BinaryStream& BinaryFileStream::Write(const char* data, size_t size)
	{
		m_File.write(data, size);
		m_Tell = m_File.tellg();
		if (m_Tell > m_Size) m_Size = m_Tell;
		return *this;
	}

	void BinaryFileStream::Close()
	{
		m_File.close();
	}

	BinaryStream& BinaryStream::Write(const std::string& value)
	{
		Write(value.size());
		return Write(value.data(), value.size());
	}
}
