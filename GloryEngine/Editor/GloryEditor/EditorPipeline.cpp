#include "EditorPipeline.h"
#include "EditorShaderData.h"

#include <BinaryStream.h>

namespace Glory::Editor
{
	EditorPipeline::EditorPipeline()
	{
		APPEND_TYPE(EditorPipeline);
	}

	EditorPipeline::EditorPipeline(std::vector<EditorShaderData>&& compiledShaders): m_EditorShaderDatas(compiledShaders)
	{
		APPEND_TYPE(EditorPipeline);
	}

	EditorPipeline::~EditorPipeline()
	{
		m_EditorShaderDatas.clear();
	}

	void EditorPipeline::Serialize(BinaryStream& container) const
	{
		container.Write(m_EditorShaderDatas.size());
		for (size_t i = 0; i < m_EditorShaderDatas.size(); ++i)
		{
			m_EditorShaderDatas[i].Serialize(container);
		}
		container.Write(m_EditorPlatformShaders);
	}

	void EditorPipeline::Deserialize(BinaryStream& container)
	{
		size_t numShaders;
		container.Read(numShaders);
		if (numShaders == 0) return;
		m_EditorShaderDatas.resize(numShaders);
		for (size_t i = 0; i < m_EditorShaderDatas.size(); ++i)
		{
			m_EditorShaderDatas[i].Deserialize(container);
		}
		container.Read(m_EditorPlatformShaders);
	}
}
