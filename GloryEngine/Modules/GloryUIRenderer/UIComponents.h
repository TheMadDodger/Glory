#pragma once
#include "UIDocumentData.h"

#include <RenderData.h>
#include <Reflection.h>
#include <AssetReference.h>
#include <TextureData.h>
#include <FontData.h>
#include <MaterialData.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

REFLECTABLE_ENUM_NS(Glory, UITarget, None, CameraOverlay, WorldSpaceQuad);
REFLECTABLE_ENUM_NS(Glory, ResolutionMode, CameraScale, Fixed);
REFLECTABLE_ENUM_NS(Glory, ScrollMode, Snap, Lerp);
REFLECTABLE_ENUM_NS(Glory, ScrollEdgeMode, Clamp, Innertia);

#define CONSTRAINT(axis)\
struct axis##Constraint\
{\
    static constexpr ConstraintAxis Axis = ConstraintAxis::axis;\
    axis##Constraint() : m_Constraint(0), m_Value(0.0f), m_FinalValue(0.0f) {}\
    axis##Constraint(float value) : m_Constraint(0), m_Value(value), m_FinalValue(0.0f) {}\
\
    explicit operator float() const { return m_FinalValue; }\
    explicit operator float() { return m_FinalValue; }\
    void operator=(float value) { m_Value = value; }\
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
            m_Pivot(0.0f, 0.0f), m_Rotation(0.0f), m_Scale(1.0f, 1.0f),
            m_ParentSize(), m_Transform(glm::identity<glm::mat4>()),
            m_TransformNoScale(glm::identity<glm::mat4>()),
            m_TransformNoScaleNoPivot(glm::identity<glm::mat4>()),
            m_InteractionTransform(glm::identity<glm::mat4>())
        {}

        REFLECTABLE(UITransform,
            (XConstraint)(m_X),
            (YConstraint)(m_Y),
            (WidthConstraint)(m_Width),
            (HeightConstraint)(m_Height),
            (glm::vec2)(m_Pivot),
            (float)(m_Rotation),
            (glm::vec2)(m_Scale)
        );

        glm::vec2 m_ParentSize;
        glm::mat4 m_Transform;
        glm::mat4 m_TransformNoScale;
        glm::mat4 m_TransformNoScaleNoPivot;
        glm::mat4 m_InteractionTransform;
        glm::mat4 m_InteractionTransformNoPivot;
    };

    /** @brief UI Image renderer */
    struct UIImage
    {
        UIImage() : m_Image(0), m_Color(1.0f, 1.0f, 1.0f, 1.0f) {}

        REFLECTABLE(UIImage,
            (AssetReference<TextureData>)(m_Image),
            (glm::vec4)(m_Color)
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
            (std::string)(m_LocalizeTerm),
            (WidthConstraint)(m_Scale),
            (glm::vec4)(m_Color),
            (Alignment)(m_Alignment)
        );
        bool m_Dirty;
    };

    /** #brief UI Box renderer */
    struct UIBox
    {
        UIBox() : m_Color(1.0f, 1.0f, 1.0f, 1.0f) {}

        REFLECTABLE(UIBox,
            (glm::vec4)(m_Color)
        );
    };

    /** @brief UI Panel that crops child elements or can be used as an empty element */
    struct UIPanel
    {
        UIPanel() : m_Crop(true) {}

        REFLECTABLE(UIPanel,
            (bool)(m_Crop)
        );
    };

    /** @brief UI Interactions */
    struct UIInteraction
    {
        UIInteraction() : m_Enabled(true), m_Hovered(false), m_Down(false) {}

        REFLECTABLE(UIInteraction,
            (bool)(m_Enabled)
        );

        bool m_Hovered;
        bool m_Down;
    };

    /** @brief UI Document renderer component */
    struct UIRenderer
    {
        UIRenderer(): m_Target(UITarget::CameraOverlay),
            m_ResolutionMode(ResolutionMode::CameraScale), m_Resolution(1.0f, 1.0f),
            m_RenderDocumentID(0), m_WorldMaterial(0), m_WorldSize(1.0f, 1.0f),
            m_InputEnabled(true), m_CursorPos(0.0f, 0.0f), m_CursorScrollDelta(0.0f, 0.0f),
            m_CursorDown(false), m_IsDirty(false)
        {}

        REFLECTABLE(UIRenderer,
            (AssetReference<UIDocumentData>)(m_Document),
            (UITarget)(m_Target),
            (ResolutionMode)(m_ResolutionMode),
            (glm::vec2)(m_Resolution),
            (AssetReference<MaterialData>)(m_WorldMaterial),
            (glm::vec2)(m_WorldSize),
            (bool)(m_InputEnabled)
        );

        UUID m_RenderDocumentID;
        glm::vec2 m_CursorPos;
        glm::vec2 m_CursorScrollDelta;
        bool m_CursorDown;
        bool m_IsDirty;
    };

    /** @brief Vertical container automatically moves child elements down */
    struct UIVerticalContainer
    {
        UIVerticalContainer() :
            m_AutoResizeHeight(true), m_Seperation(0.0f), m_Dirty(true) {}

        REFLECTABLE(UIVerticalContainer,
            (bool)(m_AutoResizeHeight),
            (float)(m_Seperation)
        );

        bool m_Dirty;
    };

    /** @brief Scroll view */
    struct UIScrollView
    {
        UIScrollView() : m_RequireHover(true), m_LockHorizontal(false), m_LockVertical(false), m_AutoScroll(true),
            m_ScrollSpeeds(-1.0f, -1.0f), m_StartScrollPosition(0.0f, 0.0f), m_ScrollMode(ScrollMode::Lerp),
            m_ScrollEdgeMode(ScrollEdgeMode::Innertia), m_LerpSpeed(10.0f), m_Innertia(10.0f),
            m_ScrollPosition(0.0f, 0.0f), m_DesiredScrollPosition(0.0f, 0.0f), m_MaxScroll(0.0f, 0.0f),
            m_Dirty(true) {}

        REFLECTABLE(UIScrollView,
            (bool)(m_RequireHover),
            (bool)(m_LockHorizontal),
            (bool)(m_LockVertical),
            (bool)(m_AutoScroll),
            (glm::vec2)(m_ScrollSpeeds),
            (glm::vec2)(m_StartScrollPosition),
            (ScrollMode)(m_ScrollMode),
            (ScrollEdgeMode)(m_ScrollEdgeMode),
            (float)(m_LerpSpeed),
            (float)(m_Innertia)
        );

        glm::vec2 m_ScrollPosition;
        glm::vec2 m_DesiredScrollPosition;
        glm::vec2 m_MaxScroll;
        bool m_Dirty;
    };
}
