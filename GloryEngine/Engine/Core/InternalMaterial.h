#pragma once
#include "MaterialData.h"

namespace Glory
{
    class PipelineData;

    /** @brief Internal material data */
    class InternalMaterial : public MaterialData
    {
    public:
        /** @brief Constructor */
        InternalMaterial(PipelineData* pPipeline);

        virtual PipelineData* GetPipeline(const MaterialManager&, const PipelineManager& pipelineManager) const override;
        virtual UUID GetPipelineID(const MaterialManager&) const override;

    private:
        PipelineData* m_pPipeline;
    };
}
