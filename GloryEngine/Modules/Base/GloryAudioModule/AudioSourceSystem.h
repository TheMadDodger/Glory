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
    struct AudioListener;

    class AudioSourceSystem
    {
    public:
        static void OnRemove(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);

        static void Play(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void Stop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void Pause(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void Resume(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);
        static void UpdateVolume(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent);

        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, AudioSource&)> OnPlaybackFinished;
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, AudioSource&)> OnSourceStart;
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, AudioSource&)> OnSourceStop;
    };

    class AudioListenerSystem
    {
    public:
        static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioListener& pComponent);
        static void OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioListener& pComponent);
        static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioListener& pComponent);

        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, AudioListener&)> OnListenerStart;
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, AudioListener&)> OnListenerStop;
        std::function<void(Utils::ECS::EntityRegistry*, Utils::ECS::EntityID, AudioListener&)> OnListenerUpdate;
    };
}
