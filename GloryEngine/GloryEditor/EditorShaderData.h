#pragma once
#include <string>
#include <vector>
#include <UUID.h>

namespace Glory::Editor
{
	class EditorShaderData
	{
	public:
		EditorShaderData(UUID uuid);
		virtual ~EditorShaderData();

	public:
		std::vector<uint32_t>::const_iterator Begin();
		std::vector<uint32_t>::const_iterator End();

		const uint32_t* Data() const;
		size_t Size() const;
		UUID GetUUID() const;

	private:
		friend class EditorShaderProcessor;
		std::vector<uint32_t> m_ShaderData;
		UUID m_UUID;
	};
}
