#include "MaterialPropertyInfo.h"

namespace Glory
{
	MaterialPropertyInfo::MaterialPropertyInfo(): m_PropertyDisplayName(""), m_PropertyShaderName(""), m_TypeHash(0),
		m_Size(0), m_Offset(0), m_IsResource(false), m_Flags(0)
	{
	}
	MaterialPropertyInfo::MaterialPropertyInfo(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, size_t offset, uint32_t flags)
		: m_PropertyDisplayName(displayName), m_PropertyShaderName(shaderName), m_TypeHash(typeHash),
		m_Size(size), m_Offset(offset), m_IsResource(false), m_Flags(flags)
	{
	}

	MaterialPropertyInfo::MaterialPropertyInfo(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t offset, uint32_t flags)
		: m_PropertyDisplayName(displayName), m_PropertyShaderName(shaderName), m_TypeHash(typeHash),
		m_Size(sizeof(UUID)), m_Offset(offset), m_IsResource(true), m_Flags(flags)
	{
	}

	const std::string& MaterialPropertyInfo::DisplayName() const
	{
		return m_PropertyDisplayName;
	}

	const std::string& MaterialPropertyInfo::ShaderName() const
	{
		return m_PropertyShaderName;
	}

	const uint32_t MaterialPropertyInfo::TypeHash() const
	{
		return m_TypeHash;
	}

	const size_t MaterialPropertyInfo::Size() const
	{
		return m_Size;
	}

	const size_t MaterialPropertyInfo::Offset() const
	{
		return m_Offset;
	}

	const size_t MaterialPropertyInfo::EndOffset() const
	{
		return m_Offset + m_Size;
	}

	bool MaterialPropertyInfo::IsResource() const
	{
		return m_IsResource;
	}

	uint32_t MaterialPropertyInfo::Flags() const
	{
		return m_Flags;
	}

	bool MaterialPropertyInfo::Write(std::vector<char>& buffer, const void* data)
	{
		if (m_IsResource) return false;
		if (buffer.size() <= m_Offset + m_Size) buffer.resize(m_Offset + m_Size);
		memcpy(&buffer[m_Offset], data, m_Size);
		return true;
	}

	void MaterialPropertyInfo::Reserve(std::vector<char>& buffer)
	{
		buffer.resize(m_Offset + m_Size);
	}
}
