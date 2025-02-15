#pragma once
#include <fstream>
#include <filesystem>

namespace Glory
{
	namespace Utils
	{
		struct BitSet;
	}

	class BinaryStream
	{
	public:
		enum class Relative
		{
			Start,
			Current,
			End
		};

		template<typename T>
		BinaryStream& Write(const T& value)
		{
			return Write(reinterpret_cast<const char*>(&value), sizeof(T));
		}

		template<typename T>
		BinaryStream& Write(const std::vector<T>& value)
		{
			return Write(value.size()).
				Write(reinterpret_cast<const char*>(value.data()), sizeof(T)*value.size());
		}

		BinaryStream& Write(const std::string& value);
		BinaryStream& Write(const std::vector<std::string>& value);
		BinaryStream& Write(const Utils::BitSet& value);

		virtual void Seek(size_t offset, Relative relative = Relative::Start) = 0;
		virtual size_t Tell() const = 0;
		virtual size_t Size() const = 0;
		virtual BinaryStream& Write(const char* data, size_t size) = 0;
		BinaryStream& Write(const void* data, size_t size);
		virtual void Close() = 0;

		template<typename T>
		BinaryStream& Read(T& out)
		{
			return Read(reinterpret_cast<char*>(&out), sizeof(T));
		}

		template<typename T>
		BinaryStream& Read(std::vector<T>& out)
		{
			size_t size;
			Read(size);
			out.resize(size);
			return Read(reinterpret_cast<char*>(out.data()), size*sizeof(T));
		}

		BinaryStream& Read(std::vector<std::string>& out);
		BinaryStream& Read(std::string& value);
		BinaryStream& Read(Utils::BitSet& value);

		BinaryStream& Read(void* out, size_t size);
		virtual BinaryStream& Read(char* out, size_t size) = 0;

		BinaryStream& Read(std::vector<char>& buffer, size_t size = 0);

		virtual bool Eof() = 0;
	};

	class BinaryMemoryStream : public BinaryStream
	{
	public:
		BinaryMemoryStream(std::vector<char>& data);
		BinaryMemoryStream(const char* data, size_t size);

		virtual void Seek(size_t offset, Relative relative = Relative::Start) override;
		virtual size_t Tell() const override;
		virtual size_t Size() const override;
		virtual BinaryStream& Write(const char* data, size_t size) override;
		virtual void Close() override;
		virtual BinaryStream& Read(char* out, size_t size) override;
		virtual bool Eof() override;

	private:
		const char* m_Data;
		size_t m_Size;
		size_t m_Tell;
	};

	class BinaryFileStream : public BinaryStream
	{
	public:
		BinaryFileStream(const std::filesystem::path& path, bool read=false, bool trunc=true);
		virtual ~BinaryFileStream();

		void Seek(size_t offset, Relative relative = Relative::Start) override;
		size_t Tell() const override;
		size_t Size() const override;
		BinaryStream& Write(const char* data, size_t size) override;
		BinaryStream& Read(char* out, size_t size) override;
		void Close() override;

		virtual bool Eof() override;

	private:
		std::fstream m_File;
		size_t m_Size;
		size_t m_Tell;
	};
}
