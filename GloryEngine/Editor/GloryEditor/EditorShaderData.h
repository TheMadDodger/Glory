#pragma once
#include "GloryEditor.h"

#include <UUID.h>
#include <GraphicsEnums.h>

#include <string>
#include <vector>
#include <spirv_cross/spirv_cross.hpp>

namespace Glory
{
	class MaterialData;
	class PipelineData;
	class BinaryStream;
}

namespace Glory::Editor
{
	class EditorShaderData
	{
	public:
		GLORY_EDITOR_API EditorShaderData();
		GLORY_EDITOR_API EditorShaderData(UUID uuid);
		virtual GLORY_EDITOR_API ~EditorShaderData();

	public:
		GLORY_EDITOR_API std::vector<uint32_t>::const_iterator Begin();
		GLORY_EDITOR_API std::vector<uint32_t>::const_iterator End();

		GLORY_EDITOR_API const uint32_t* Data() const;
		GLORY_EDITOR_API size_t Size() const;
		GLORY_EDITOR_API UUID GetUUID() const;

		GLORY_EDITOR_API void LoadIntoMaterial(MaterialData* pMaterial) const;
		GLORY_EDITOR_API void LoadIntoPipeline(PipelineData* pMaterial) const;

		void Serialize(BinaryStream& container) const;
		void Deserialize(BinaryStream& container);

	private:
		friend class EditorPipelineManager;
		friend class MaterialEditor;
		friend class MaterialInstanceEditor;
		ShaderType m_ShaderType;
		std::vector<uint32_t> m_ShaderData;
		UUID m_UUID;

		struct PropertyInfo
		{
			GLORY_EDITOR_API PropertyInfo();
			GLORY_EDITOR_API PropertyInfo(const std::string& name, uint32_t typeHash);
			std::string m_Name;
			uint32_t m_TypeHash;
		};
		std::vector<std::string> m_SamplerNames;
		std::vector<PropertyInfo> m_PropertyInfos;

		std::vector<std::string> m_Features;
	};
}
