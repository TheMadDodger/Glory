#pragma once
#include <string>
#include <vector>
#include <UUID.h>
#include <spirv_cross.hpp>
#include <MaterialData.h>

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

		void LoadIntoMaterial(MaterialData* pMaterial);

	private:
		friend class EditorShaderProcessor;
		std::vector<uint32_t> m_ShaderData;
		UUID m_UUID;

		struct PropertyInfo
		{
			PropertyInfo(const std::string& name, size_t typeHash);
			std::string m_Name;
			size_t m_TypeHash;
		};
		std::vector<std::string> m_SamplerNames;
		std::vector<PropertyInfo> m_PropertyInfos;
	};
}
