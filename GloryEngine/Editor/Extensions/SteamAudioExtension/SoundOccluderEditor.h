#pragma once
#include <EntityComponentEditor.h>
#include <SoundComponents.h>

namespace Glory::Editor
{
    class SoundOccluderEditor : public EntityComponentEditor<SoundOccluderEditor, SoundOccluder>
    {
    public:
        SoundOccluderEditor();
        virtual ~SoundOccluderEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;

        Editor* m_pSoundMaterialEditor;
    };
}