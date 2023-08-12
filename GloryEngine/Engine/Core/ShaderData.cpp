#include "ShaderData.h"

namespace Glory
{
	std::vector<uint32_t>::const_iterator ShaderData::Begin()
	{
		return m_ShaderData.begin();
	}

	std::vector<uint32_t>::const_iterator ShaderData::End()
	{
		return m_ShaderData.end();
	}

	const uint32_t* ShaderData::Data()
	{
		return m_ShaderData.data();
	}

	size_t ShaderData::Size()
	{
		return m_ShaderData.size();
	}
}