#pragma once
#include "MaterialData.h"

namespace Glory
{
    class PipelineData;

    class InternalMaterial : public MaterialData
    {
    public:
        InternalMaterial(PipelineData* pPipeline);

        virtual PipelineData* GetPipeline(const MaterialManager&, const PipelineManager& pipelineManager) const override;
        virtual UUID GetPipelineID(const MaterialManager&) const override;

    private:
        PipelineData* m_pPipeline;
    };
}
