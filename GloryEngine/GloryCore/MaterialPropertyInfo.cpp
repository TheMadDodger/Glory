#include "MaterialPropertyInfo.h"

namespace Glory
{
	MaterialPropertyInfo::MaterialPropertyInfo(const std::string& displayName, const std::string& shaderName, size_t typeHash, size_t size, size_t offset, uint32_t flags)
		: m_PropertyDisplayName(displayName), m_PropertyShaderName(shaderName), m_TypeHash(typeHash), m_Size(size), m_Offset(offset), m_Flags(flags)
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

	const size_t MaterialPropertyInfo::TypeHash() const
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

	uint32_t MaterialPropertyInfo::Flags() const
	{
		return m_Flags;
	}

	void MaterialPropertyInfo::Reserve(std::vector<char>& buffer)
	{
		buffer.resize(m_Offset + m_Size);
	}
}
