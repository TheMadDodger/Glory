#pragma once
#include <Reflection.h>

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
}
