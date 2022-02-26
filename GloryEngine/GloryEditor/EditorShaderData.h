#pragma once
#include <string>
#include <vector>

namespace Glory::Editor
{
	class EditorShaderData
	{
	public:
		std::vector<uint32_t>::const_iterator Begin();
		std::vector<uint32_t>::const_iterator End();

		const uint32_t* Data();
		size_t Size();

	private:
		friend class EditorShaderProcessor;
		std::vector<uint32_t> m_ShaderData;
	};
}
