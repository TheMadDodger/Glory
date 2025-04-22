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
    struct StringTableLoader;
    struct Localize;

    class StringTableLoaderSystem
    {
    public:
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, StringTableLoader& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, StringTableLoader& pComponent);
        static void GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references);
    };

    class LocalizeSystem
    {
    public:
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Localize& pComponent);
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Localize& pComponent);
    };
}