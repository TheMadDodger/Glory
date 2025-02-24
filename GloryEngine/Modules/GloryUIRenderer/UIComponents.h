#pragma once
#include "UIDocumentData.h"

#include <Reflection.h>
#include <AssetReference.h>

#include <glm/glm.hpp>

namespace Glory
{
    struct UITransform
    {
        REFLECTABLE(UITransform,
            (glm::vec4)(m_Rect));
    };

    struct UIPanel
    {
        REFLECTABLE(UIPanel,
            (glm::vec4)(m_Rect));
    };

    struct UIText
    {
        REFLECTABLE(UIText,
            (std::string)(m_Text)
        );
    };

    struct UIRenderer
    {
        REFLECTABLE(UIRenderer,
            (AssetReference<UIDocumentData>)(m_Document)
        );
    };
}
