#pragma once
#include <EntityID.h>
#include <UUID.h>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
    class BaseTypeView;
}

namespace Glory
{
    struct Localize;

    class LocalizeSystem
    {
    public:
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Localize& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Localize& pComponent);
        static void GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references);
    };
}