#pragma once
#include <EntityID.h>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    struct AudioSource;

    class AudioSourceSystem
    {
    public:
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);

        static void Play(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
    };
}
