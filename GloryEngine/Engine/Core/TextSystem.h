#pragma once
#include <EntityID.h>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    struct TextComponent;

    class TextSystem
    {
    public:
        static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent);
    };
}