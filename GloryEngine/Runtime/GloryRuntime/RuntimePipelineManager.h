#pragma once
#include <PipelineManager.h>

namespace Glory
{
	/* @brief Runtime pipeline manager */
    class RuntimePipelineManager : public PipelineManager
    {
	public:
		/** @brief Constructor */
		RuntimePipelineManager(Engine* pEngine);
		/** @brief Destructor */
		virtual ~RuntimePipelineManager() = default;

	private:
		virtual PipelineData* GetPipelineData(UUID pipelineID) const override;

		/** @brief Get all platform compiled shaders for a pipeline */
		virtual const std::vector<FileData>& GetPipelineCompiledShaders(UUID pipelineID) const override;
		/** @brief Get shader types */
		virtual const std::vector<ShaderType>& GetPipelineShaderTypes(UUID pipelineID) const override;

		/** @brief Add a compiled shader */
		virtual void AddShader(FileData* pShader) override;

	private:
		std::map<UUID, std::vector<FileData>> m_Shaders;
		std::map<UUID, std::vector<ShaderType>> m_ShaderTypes;
    };
}
