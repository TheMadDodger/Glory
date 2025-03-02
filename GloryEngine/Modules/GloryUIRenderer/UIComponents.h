#pragma once
#include "UIDocumentData.h"

#include <Reflection.h>
#include <AssetReference.h>
#include <TextureData.h>

#include <glm/glm.hpp>

REFLECTABLE_ENUM_NS(Glory, UITarget, CameraOverlay);
REFLECTABLE_ENUM_NS(Glory, ResolutionMode, CameraScale, Fixed);

namespace Glory
{
    /** @brief Transform for UI components */
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

    /** @brief UI Image renderer */
    struct UIImage
    {
    AssetReference<TextureData> m_Image;
    typedef UIImage TypeName;
    public:
        static const TypeData* GetTypeData() {
            static const char* typeNameString = "UIImage";
            static const uint32_t TYPE_HASH = Reflect::Hash<UIImage>();
            static const int NUM_ARGS = 1;
            static const FieldData pFields[] = { FieldData(Reflect::Hash(typeid(m_Image)), "m_Image", "AssetReference<TextureData>", ((::size_t) & reinterpret_cast<char const volatile&>((((TypeName*)0)->m_Image))), sizeof(AssetReference<TextureData>)), }; static const TypeData pTypeData = TypeData(typeNameString, pFields, uint32_t(CustomTypeHash::Struct), TYPE_HASH, NUM_ARGS, -1, 0); return &pTypeData;
    } static int DataBufferOffset() {
        return -1;
    } static int DataBufferSize() {
        return 0;
    };
    };

    /** @brief UI Text renderer */
    struct UIText
    {
        UIText() : m_Text("For Glory!"), m_Dirty(true) {}

        REFLECTABLE(UIText,
            (std::string)(m_Text)
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
