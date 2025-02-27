#pragma once
#include "UIDocumentData.h"

#include <Reflection.h>
#include <AssetReference.h>

#include <glm/glm.hpp>

REFLECTABLE_ENUM_NS(Glory, UITarget, CameraOverlay);
REFLECTABLE_ENUM_NS(Glory, ResolutionMode, CameraScale, Fixed);

namespace Glory
{
    struct UITransform
    {
        UITransform() : m_Rotation(0.0f), m_IsDirty(false) {}
        REFLECTABLE(UITransform,
            (glm::vec4)(m_Rect),
            (float)(m_Rotation)
        );

        glm::mat4 m_Transform;
        bool m_IsDirty;
    };

    struct UIPanel
    {
        REFLECTABLE(UIPanel,
            (glm::vec4)(m_Rect)
        );
    };

    struct UIText
    {
        REFLECTABLE(UIText,
            (std::string)(m_Text)
        );
    };

    struct UIRenderer
    {
        UIRenderer(): m_Target(UITarget::CameraOverlay),
            m_ResolutionMode(ResolutionMode::CameraScale), m_Resolution(1.0f, 1.0f)
        {}

        REFLECTABLE(UIRenderer,
            (AssetReference<UIDocumentData>)(m_Document),
            (UITarget)(m_Target),
            (ResolutionMode)(m_ResolutionMode),
            (glm::vec2)(m_Resolution)
        );
    };
}
