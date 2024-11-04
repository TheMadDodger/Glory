#include "BinaryStream.h"

namespace Glory
{
	BinaryFileStream::BinaryFileStream(const std::filesystem::path& path, bool read, bool trunc):
		m_File(), m_Size(0), m_Tell(0)
	{
		if (read)
			m_File.open(path.string(), std::fstream::in | std::ios::binary | std::ios::ate);
		else
			m_File.open(path.string(), std::fstream::out | std::ios::binary | (trunc ? std::fstream::trunc : std::ios_base::app));

		m_Size = m_File.tellg();
		m_File.seekg(0, std::ios_base::beg);
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

	BinaryStream& BinaryFileStream::Read(char* out, size_t size)
	{
		m_File.read(out, size);
		m_Tell = m_File.tellg();
		return *this;
	}

	void BinaryFileStream::Close()
	{
		m_File.close();
	}

	bool BinaryFileStream::Eof()
	{
		return m_Tell >= m_Size;
	}

	BinaryStream& BinaryStream::Write(const std::string& value)
	{
		Write(value.size());
		return Write(value.data(), value.size());
	}

	BinaryStream& BinaryStream::Write(const std::vector<std::string>& value)
	{
		Write(value.size());
		for (size_t i = 0; i < value.size(); ++i)
		{
			Write(value[i]);
		}
		return *this;
	}

	BinaryStream& BinaryStream::Write(const void* data, size_t size)
	{
		return Write(reinterpret_cast<const char*>(data), size);
	}

	BinaryStream& BinaryStream::Read(std::vector<std::string>& out)
	{
		size_t size;
		Read(size);
		out.resize(size);
		for (size_t i = 0; i < size; ++i)
			Read(out[i]);
		return *this;
	}

	BinaryStream& BinaryStream::Read(std::string& value)
	{
		size_t size = 0;
		Read(size);
		value.resize(size);
		return Read(value.data(), size);
	}

	BinaryStream& BinaryStream::Read(void* out, size_t size)
	{
		return Read(reinterpret_cast<char*>(out), size);
	}

	void BinaryStream::Read(std::vector<char>& buffer, size_t size)
	{
		if (size == 0)
			size = Size() - Tell();
		buffer.resize(size);
		Read(buffer.data(), buffer.size());
	}

	BinaryMemoryStream::BinaryMemoryStream(std::vector<char>& data):
		BinaryMemoryStream(data.data(), data.size())
	{
	}

	BinaryMemoryStream::BinaryMemoryStream(const char* data, size_t size):
		m_Data(data), m_Size(size), m_Tell(0)
	{
	}

	void BinaryMemoryStream::Seek(size_t offset, Relative relative)
	{
		switch (relative)
		{
		case Glory::BinaryStream::Relative::Start:
			m_Tell = offset;
			break;
		case Glory::BinaryStream::Relative::Current:
			m_Tell = m_Tell + offset;
			break;
		case Glory::BinaryStream::Relative::End:
			m_Tell = m_Size - offset;
			break;
		default:
			break;
		}
	}

	size_t BinaryMemoryStream::Tell() const
	{
		return m_Tell;
	}

	size_t BinaryMemoryStream::Size() const
	{
		return m_Size;
	}

	BinaryStream& BinaryMemoryStream::Write(const char* data, size_t size)
	{
		return *this;
	}

	void BinaryMemoryStream::Close()
	{
	}

	BinaryStream& BinaryMemoryStream::Read(char* out, size_t size)
	{
		std::memcpy(out, &m_Data[m_Tell], size);
		m_Tell += size;
		return *this;
	}

	bool BinaryMemoryStream::Eof()
	{
		return m_Tell >= m_Size;
	}
}
