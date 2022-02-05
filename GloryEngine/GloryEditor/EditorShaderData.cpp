#include "EditorShaderData.h"

namespace Glory::Editor
{
	std::vector<uint32_t>::const_iterator EditorShaderData::Begin()
	{
		return m_ShaderData.begin();
	}

	std::vector<uint32_t>::const_iterator EditorShaderData::End()
	{
		return m_ShaderData.end();
	}

	const uint32_t* EditorShaderData::Data()
	{
		return m_ShaderData.data();
	}

	size_t EditorShaderData::Size()
	{
		return m_ShaderData.size();
	}
}