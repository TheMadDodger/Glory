#pragma once
#include "Editor.h"
#include "EditableResource.h"

#include <PipelineData.h>

namespace Glory::Editor
{
    /** @brief Pipeline editor */
    class PipelineEditor : public EditorTemplate<PipelineEditor, YAMLResource<PipelineData>>
    {
    public:
        /** @brief Constructor */
        PipelineEditor();
        /** @brief Destructor */
        virtual ~PipelineEditor();

    private:
        virtual bool OnGUI() override;
        virtual void Initialize() override;
    };
}
