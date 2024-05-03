#pragma once
#include <string>
#include <vector>
#include "UUID.h"
#include "Resource.h"

namespace Glory
{
	struct MaterialPropertyInfo
	{
	public:
		MaterialPropertyInfo();
		MaterialPropertyInfo(const MaterialPropertyInfo& other);
		MaterialPropertyInfo(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, size_t offset, uint32_t flags);
		MaterialPropertyInfo(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t offset, uint32_t flags);

		const std::string& DisplayName() const;
		const std::string& ShaderName() const;
		const uint32_t TypeHash() const;
		const size_t Size() const;
		const size_t Offset() const;
		const size_t EndOffset() const;
		bool IsResource() const;
		uint32_t Flags() const;

		template<typename T>
		bool Read(const std::vector<char> buffer, T& data) const
		{
			if (m_IsResource) return false;
			if (sizeof(T) > m_Size) return false;
			if (buffer.size() > m_Offset + m_Size) return false;
			memcpy((void*)&data, &buffer[m_Offset], m_Size);
			return true;
		}

		template<typename T>
		bool Write(std::vector<char>& buffer, const T& data)
		{
			if (m_IsResource) return false;
			if (sizeof(T) > m_Size) return false;
			if (buffer.size() <= m_Offset + m_Size) buffer.resize(m_Offset + m_Size);
			memcpy(&buffer[m_Offset], (void*)&data, m_Size);
			return true;
		}

		bool Write(std::vector<char>& buffer, const void* data);
		void Reserve(std::vector<char>& buffer);

	private:
		friend class MaterialData;
		friend class PipelineData;
		std::string m_PropertyDisplayName;
		std::string m_PropertyShaderName;
		uint32_t m_TypeHash;
		size_t m_Size;
		size_t m_Offset;
		bool m_IsResource;
		uint32_t m_Flags;
	};
}
