#pragma once
#include "Editor.h"
#include "EditableResource.h"

#include <PipelineData.h>

namespace Glory::Editor
{
    class PipelineEditor : public EditorTemplate<PipelineEditor, YAMLResource<PipelineData>>
    {
    public:
        PipelineEditor();
        virtual ~PipelineEditor();

    private:
        virtual bool OnGUI() override;
        virtual void Initialize() override;
    };
}
