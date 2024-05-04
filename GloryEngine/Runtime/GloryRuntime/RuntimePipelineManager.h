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
    };
}
