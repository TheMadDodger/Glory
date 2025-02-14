#pragma once
#include "UUID.h"
#include "FileData.h"

#include <string>

namespace Glory
{
	class Engine;
	class PipelineData;
	enum class ShaderType;

	/** @brief Pipeline manager */
	class PipelineManager
	{
	public:
		/** @brief Constructor */
		PipelineManager(Engine* pEngine);
		/** @brief Destructor */
		virtual ~PipelineManager() = default;

		/** @brief Get a pipeline by ID
		 * @param pipelineID ID of the pipeline to get
		 */
		virtual PipelineData* GetPipelineData(UUID pipelineID) const = 0;

		/** @brief Get all platform compiled shaders for a pipeline */
		virtual const std::vector<FileData>& GetPipelineCompiledShaders(UUID pipelineID) const { return {}; };
		/** @brief Get shader types */
		virtual const std::vector<ShaderType>& GetPipelineShaderTypes(UUID pipelineID) const { return {}; };

	protected:
		Engine* m_pEngine;
	};
}
