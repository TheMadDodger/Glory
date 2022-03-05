#include "MaterialPropertyData.h"

namespace Glory
{
	MaterialPropertyData::MaterialPropertyData()
		: m_PropertyName("empty property"), m_PropertyData("null"), m_Flags(0) {}

	MaterialPropertyData::MaterialPropertyData(const MaterialPropertyData& other)
		: m_PropertyName(other.m_PropertyName), m_PropertyData(other.m_PropertyData), m_Flags(other.m_Flags) {}

	MaterialPropertyData::MaterialPropertyData(const std::string& name) : m_PropertyName(name), m_PropertyData("null"), m_Flags(0)
	{
	}

	MaterialPropertyData::MaterialPropertyData(const std::string& name, const std::any& data, uint32_t flags)
		: m_PropertyName(name), m_PropertyData(data), m_Flags(flags) {}

	const std::string& MaterialPropertyData::Name() const
	{
		return m_PropertyName;
	}

	const std::type_index MaterialPropertyData::Type() const
	{
		return m_PropertyData.type();
	}

	uint32_t MaterialPropertyData::Flags() const
	{
		return m_Flags;
	}

	std::any& MaterialPropertyData::Data()
	{
		return m_PropertyData;
	}
}
