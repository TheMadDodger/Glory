#pragma once
#include "AudioComponents.h"

#include <ComponentManager.h>

namespace Glory
{
    struct AudioSource;
    struct AudioListener;
    class AudioModule;
    class Resources;

    class AudioSourceManager : public Utils::ECS::ComponentManager<AudioSource>
    {
    public:
        AudioSourceManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity=32);
        virtual ~AudioSourceManager();

    public:
        void OnRemoveImpl(Utils::ECS::EntityID entity, AudioSource& pComponent);
        void OnStartImpl(Utils::ECS::EntityID entity, AudioSource& pComponent);
        void OnValidateImpl(Utils::ECS::EntityID entity, AudioSource& pComponent);
        void OnStopImpl(Utils::ECS::EntityID entity, AudioSource& pComponent);
        void OnUpdateImpl(Utils::ECS::EntityID entity, AudioSource& pComponent, float);

        void Play(Utils::ECS::EntityID entity, AudioSource& pComponent);
        void Stop(Utils::ECS::EntityID entity, AudioSource& pComponent);
        void Pause(Utils::ECS::EntityID entity, AudioSource& pComponent);
        void Resume(Utils::ECS::EntityID entity, AudioSource& pComponent);
        void UpdateVolume(Utils::ECS::EntityID entity, AudioSource& pComponent);

        void GetReferencesImpl(std::vector<UUID>& references) const;

        void OnDeserialize(Utils::BinaryStream&) override;
        void OnCopy(AudioSource& audio) override;

    private:
        virtual void OnInitialize() override;

    private:
        friend class AudioModule;
        AudioModule* m_pAudioModule;
        Resources* m_pResources;
    };

    class AudioListenerManager : public Utils::ECS::ComponentManager<AudioListener>
    {
    public:
        AudioListenerManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity=1);
        virtual ~AudioListenerManager();

    public:
        void OnStartImpl(Utils::ECS::EntityID entity, AudioListener& pComponent);
        void OnStopImpl(Utils::ECS::EntityID entity, AudioListener& pComponent);
        void OnUpdateImpl(Utils::ECS::EntityID entity, AudioListener& pComponent, float);

    private:
        virtual void OnInitialize() override;

    private:
        friend class AudioModule;
        AudioModule* m_pAudioModule;
    };
}
