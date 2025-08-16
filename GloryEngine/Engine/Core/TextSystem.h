#pragma once
#include "UUID.h"

#include <vector>
#include <EntityID.h>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
    class BaseTypeView;
}

namespace Glory
{
    struct TextComponent;

    class TextSystem
    {
    public:
        static void OnDisableDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent);
        static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent);
        static void GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references);
    };
}