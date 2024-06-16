#pragma once
#include <EntityID.h>

#include <functional>

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
        static void OnRemove(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);

        static void Play(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void Stop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);

        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, uint32_t)> OnPlaybackFinished;
    };
}
