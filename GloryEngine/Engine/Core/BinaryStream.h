#pragma once
#include <fstream>
#include <filesystem>

namespace Glory
{
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

		BinaryStream& Write(const std::string& value);

		virtual void Seek(size_t offset, Relative relative = Relative::Start) = 0;
		virtual size_t Tell() const = 0;
		virtual size_t Size() const = 0;
		virtual BinaryStream& Write(const char* data, size_t size) = 0;
		BinaryStream& Write(const void* data, size_t size);
		virtual void Close() = 0;
	};

	class BinaryFileStream : public BinaryStream
	{
	public:
		BinaryFileStream(const std::filesystem::path& path);
		virtual ~BinaryFileStream();

		void Seek(size_t offset, Relative relative = Relative::Start) override;
		size_t Tell() const override;
		size_t Size() const override;
		BinaryStream& Write(const char* data, size_t size) override;
		void Close() override;

	private:
		std::fstream m_File;
		size_t m_Size;
		size_t m_Tell;
	};
}
