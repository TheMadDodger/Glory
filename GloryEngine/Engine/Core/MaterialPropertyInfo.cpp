#include "MaterialPropertyInfo.h"

namespace Glory
{
	MaterialPropertyInfo::MaterialPropertyInfo(): m_PropertyDisplayName(""), m_PropertyShaderName(""), m_TypeHash(0),
		m_Size(0), m_Offset(0), m_TextureType(TT_None), m_Flags(0)
	{
	}

	MaterialPropertyInfo::MaterialPropertyInfo(const MaterialPropertyInfo& other):
		m_PropertyDisplayName(other.m_PropertyDisplayName), m_PropertyShaderName(other.m_PropertyShaderName), m_TypeHash(other.m_TypeHash),
		m_Size(other.m_Size), m_Offset(other.m_Offset), m_TextureType(other.m_TextureType), m_Flags(other.m_Flags)
	{
	}

	MaterialPropertyInfo::MaterialPropertyInfo(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, size_t offset, uint32_t flags)
		: m_PropertyDisplayName(displayName), m_PropertyShaderName(shaderName), m_TypeHash(typeHash),
		m_Size(size), m_Offset(offset), m_TextureType(TT_None), m_Flags(flags)
	{
	}

	MaterialPropertyInfo::MaterialPropertyInfo(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t offset, TextureType textureType, uint32_t flags)
		: m_PropertyDisplayName(displayName), m_PropertyShaderName(shaderName), m_TypeHash(typeHash),
		m_Size(sizeof(UUID)), m_Offset(offset), m_TextureType(textureType), m_Flags(flags)
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
		return m_TextureType;
	}

	TextureType MaterialPropertyInfo::GetTextureType() const
	{
		return m_TextureType;
	}

	uint32_t MaterialPropertyInfo::Flags() const
	{
		return m_Flags;
	}

	void* MaterialPropertyInfo::Address(std::vector<char>& buffer)
	{
		if (m_TextureType) return nullptr;
		if (buffer.size() <= m_Offset + m_Size) buffer.resize(m_Offset + m_Size);
		return &buffer[m_Offset];
	}

	bool MaterialPropertyInfo::Write(std::vector<char>& buffer, const void* data)
	{
		if (m_TextureType) return false;
		if (buffer.size() <= m_Offset + m_Size) buffer.resize(m_Offset + m_Size);
		memcpy(&buffer[m_Offset], data, m_Size);
		return true;
	}

	void MaterialPropertyInfo::Reserve(std::vector<char>& buffer)
	{
		buffer.resize(m_Offset + m_Size);
	}
}
