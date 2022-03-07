#pragma once
#include <string>
#include <vector>

namespace Glory
{
	struct MaterialPropertyInfo
	{
	public:
		MaterialPropertyInfo(const std::string& displayName, const std::string& shaderName, size_t typeHash, size_t size, size_t offset, uint32_t flags = 0);

		const std::string& DisplayName() const;
		const std::string& ShaderName() const;
		const size_t TypeHash() const;
		const size_t Size() const;
		const size_t Offset() const;
		const size_t EndOffset() const;
		uint32_t Flags() const;

		template<typename T>
		bool Read(const std::vector<char> buffer, const T& data) const
		{
			if (sizeof(T) > m_Size) return false;
			if (buffer.size() <= m_Offset + m_Size) return false;
			memcpy((void*)&outData, &buffer[m_Offset], m_Size);
			return true;
		}

		template<typename T>
		bool Write(std::vector<char>& buffer, const T& data)
		{
			if (sizeof(T) > m_Size) return false;
			if (buffer.size() <= m_Offset + m_Size) buffer.resize(m_Offset + m_Size);
			memcpy(&buffer[m_Offset], (void*)data, m_Size);
			return true;
		}

		void Reserve(std::vector<char>& buffer);

	private:
		friend class MaterialLoaderModule;
		std::string m_PropertyDisplayName;
		std::string m_PropertyShaderName;
		size_t m_TypeHash;
		size_t m_Size;
		size_t m_Offset;
		uint32_t m_Flags;
	};
}
