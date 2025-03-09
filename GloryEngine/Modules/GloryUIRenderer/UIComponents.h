#pragma once
#include "UIDocumentData.h"

#include <RenderData.h>
#include <Reflection.h>
#include <AssetReference.h>
#include <TextureData.h>
#include <FontData.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

REFLECTABLE_ENUM_NS(Glory, UITarget, CameraOverlay);
REFLECTABLE_ENUM_NS(Glory, ResolutionMode, CameraScale, Fixed);

#define CONSTRAINT(axis)\
struct axis##Constraint\
{\
    static constexpr ConstraintAxis Axis = ConstraintAxis::axis;\
    axis##Constraint() : m_Constraint(0), m_Value(0.0f), m_FinalValue(0.0f) {}\
    axis##Constraint(float value) : m_Constraint(0), m_Value(value), m_FinalValue(0.0f) {}\
\
    explicit operator float() const { return m_FinalValue; }\
    explicit operator float() { return m_FinalValue; }\
\
    REFLECTABLE(axis##Constraint,\
        (uint32_t)(m_Constraint),\
        (float)(m_Value)\
    );\
    float m_FinalValue;\
};

namespace Glory
{
    /** @brief Constraint axis */
    enum ConstraintAxis
    {
        X,
        Y,
        Width,
        Height
    };

    /** @brief X Constraint */
    CONSTRAINT(X);
    /** @brief Y Constraint */
    CONSTRAINT(Y);
    /** @brief Width Constraint */
    CONSTRAINT(Width);
    /** @brief Height Constraint */
    CONSTRAINT(Height);

    /** @brief Transform for UI components */
    struct UITransform
    {
        UITransform() : m_X(0.0f), m_Y(0.0f), m_Width(100.0f), m_Height(100.0f),
            m_Pivot(0.0f, 0.0f), m_Depth(0.0f), m_Rotation(0.0f),
            m_ParentSize(), m_Transform(glm::identity<glm::mat4>()),
            m_TransformNoScale(glm::identity<glm::mat4>()),
            m_TransformNoScaleNoPivot(glm::identity<glm::mat4>()),
            m_IsDirty(false) {}

        REFLECTABLE(UITransform,
            (XConstraint)(m_X),
            (YConstraint)(m_Y),
            (WidthConstraint)(m_Width),
            (HeightConstraint)(m_Height),
            (glm::vec2)(m_Pivot),
            (float)(m_Depth),
            (float)(m_Rotation)
        );

        glm::vec2 m_ParentSize;
        glm::mat4 m_Transform;
        glm::mat4 m_TransformNoScale;
        glm::mat4 m_TransformNoScaleNoPivot;
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
        UIText() : m_Font(0), m_Text("Hello World!"), m_Scale(1.0f),
            m_Color(1.0f, 1.0f, 1.0f, 1.0f), m_Alignment(Alignment::Left),
            m_Dirty(true) {}

        REFLECTABLE(UIText,
            (AssetReference<FontData>)(m_Font),
            (std::string)(m_Text),
            (WidthConstraint)(m_Scale),
            (glm::vec4)(m_Color),
            (Alignment)(m_Alignment)
        );
        bool m_Dirty;
    };

    struct UIPanel
    {
        REFLECTABLE(UIPanel,
            (glm::vec4)(m_Rect)
        );
    };

    /** @brief UI Document renderer component */
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
