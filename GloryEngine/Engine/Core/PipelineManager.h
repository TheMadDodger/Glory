#pragma once
#include "UUID.h"

namespace Glory
{
	class Engine;
	class PipelineData;

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

	protected:
		Engine* m_pEngine;
	};
}
