#pragma once
#include <PipelineData.h>

namespace Glory::Editor
{
	class EditorShaderData;

	class EditorPipeline : public PipelineData
	{
	public:
		EditorPipeline();
		EditorPipeline(std::vector<EditorShaderData>&& compiledShaders);
		virtual ~EditorPipeline();

		/** @brief Serialize the pipeline into a binary stream */
		void Serialize(BinaryStream& container) const override;
		/** @brief Deserialize the pipeline from a binary stream */
		void Deserialize(BinaryStream& container) override;

	private:
		friend class EditorPipelineManager;
		std::vector<EditorShaderData> m_EditorShaderDatas;
		std::vector<std::string> m_EditorPlatformShaders;
	};
}
