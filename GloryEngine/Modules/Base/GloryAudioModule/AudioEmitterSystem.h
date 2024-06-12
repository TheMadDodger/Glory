#pragma once
#include <EntityID.h>

namespace Glory::Utils::ECS
{
    class EntityRegistry;
}

namespace Glory
{
    struct AudioEmitter;

    class AudioEmitterSystem
    {
    public:
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioEmitter& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioEmitter& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioEmitter& pComponent);

        static void Play(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioEmitter& pComponent);
    };
}
