#pragma once
#include "UIDocumentData.h"

#include <RenderData.h>
#include <Reflection.h>
#include <AssetReference.h>
#include <TextureData.h>
#include <FontData.h>

#include <glm/glm.hpp>

REFLECTABLE_ENUM_NS(Glory, UITarget, CameraOverlay);
REFLECTABLE_ENUM_NS(Glory, ResolutionMode, CameraScale, Fixed);

namespace Glory
{
    /** @brief Transform for UI components */
    struct UITransform
    {
        UITransform() : m_Rect(0.0f, 0.0f, 100.0f, 100.0f),
            m_Pivot(0.0f, 0.0f), m_Depth(0.0f),
            m_Rotation(0.0f), m_IsDirty(false) {}

        REFLECTABLE(UITransform,
            (glm::vec4)(m_Rect),
            (glm::vec2)(m_Pivot),
            (float)(m_Depth),
            (float)(m_Rotation)
        );

        glm::mat4 m_Transform;
        bool m_IsDirty;
    };

    /** @brief UI Image renderer */
    struct UIImage
    {
        REFLECTABLE(UIImage,
            (AssetReference<TextureData>)(m_Image)
        );
    };

    /** @brief UI Text renderer */
    struct UIText
    {
        UIText() : m_Font(0), m_Text("Hello World!"), m_Scale(0.01f),
            m_Color(1.0f, 1.0f, 1.0f, 1.0f), m_Alignment(Alignment::Left),
            m_WrapWidth(0.0f), m_Dirty(true) {}

        REFLECTABLE(UIText,
            (AssetReference<FontData>)(m_Font),
            (std::string)(m_Text),
            (float)(m_Scale),
            (glm::vec4)(m_Color),
            (Alignment)(m_Alignment),
            (float)(m_WrapWidth)
        );
        bool m_Dirty;
    };

    struct UIPanel
    {
        REFLECTABLE(UIPanel,
            (glm::vec4)(m_Rect)
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
