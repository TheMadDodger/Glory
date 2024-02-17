#pragma once
#include "GloryEditor.h"

#include <TextureData.h>

namespace Glory::Editor
{
    class EditorTextureData : public TextureData
    {
    public:
        GLORY_EDITOR_API EditorTextureData(ImageData* pImageData);
        GLORY_EDITOR_API virtual ~EditorTextureData();

        GLORY_EDITOR_API virtual ImageData* GetImageData(AssetManager*) override;

    private:
        ImageData* m_pImageData;
    };
}
