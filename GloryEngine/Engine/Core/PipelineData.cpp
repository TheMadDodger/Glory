#include "PipelineData.h"

namespace Glory
{
	PipelineData::PipelineData()
	{
		APPEND_TYPE(PipelineData);
	}

	PipelineData::~PipelineData()
	{
	}

	size_t PipelineData::ShaderCount() const
	{
		return m_Shaders.size();
	}

	PipelineType PipelineData::Type() const
	{
		return m_Type;
	}

	UUID PipelineData::ShaderID(size_t index) const
	{
		return m_Shaders[index];
	}

	void PipelineData::SetPipelineType(PipelineType type)
	{
		m_Type = type;
	}

	void PipelineData::AddShader(UUID shaderID)
	{
		m_Shaders.push_back(shaderID);
	}

	void PipelineData::RemoveShaderAt(size_t index)
	{
		m_Shaders.erase(m_Shaders.begin() + index);
	}

	void PipelineData::Serialize(BinaryStream& container) const
	{
	}

	void PipelineData::Deserialize(BinaryStream& container)
	{
	}
}
