#include "AudioComponentManager.h"
#include "AudioModule.h"

#include <IEngine.h>
#include <GScene.h>
#include <SceneManager.h>
#include <TransformManager.h>

#include <EntityRegistry.h>

namespace Glory
{
	AudioSourceManager::AudioSourceManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity), m_pAudioModule(nullptr), m_pResources(nullptr)
	{
	}

	AudioSourceManager::~AudioSourceManager()
	{
	}

	void AudioSourceManager::OnRemoveImpl(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		Stop(entity, pComponent);
	}

	void AudioSourceManager::OnStartImpl(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		auto& callback = m_pAudioModule->OnSourceStart;
		if (callback) callback(m_pRegistry, entity, pComponent);

		if (!pComponent.m_AutoPlay) return;
		Play(entity, pComponent);
	}

	void AudioSourceManager::OnValidateImpl(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		UpdateVolume(entity, pComponent);
	}

	void AudioSourceManager::OnStopImpl(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		auto& callback = m_pAudioModule->OnSourceStop;
		if (callback) callback(m_pRegistry, entity, pComponent);

		Stop(entity, pComponent);
	}

	void AudioSourceManager::OnUpdateImpl(Utils::ECS::EntityID entity, AudioSource& pComponent, float)
	{
		auto& callback = m_pAudioModule->OnSourceUpdate;
		if (callback) callback(m_pRegistry, entity, pComponent);
	}

	void FinishedCallback(IEngine* pEngine, const AudioChannel& channel)
	{
		if (channel.m_UserData.m_Type != AudioChannelUDataType::Entity) return;
		GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(channel.m_UserData.sceneID());
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(channel.m_UserData.entityID());
		if (!entity.IsValid()) return;
		if (!entity.HasComponent<AudioSource>()) return;
		entity.GetComponent<AudioSource>().m_CurrentChannel = -1;
	}

	void AudioSourceManager::Play(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (!pComponent.m_Audio.GetUUID()) return;

		AudioData* pAudio = pComponent.m_Audio.Get(m_pResources);
		if (!pAudio) return;

		if (pComponent.m_AsMusic)
		{
			m_pAudioModule->PlayMusic(pAudio, pComponent.m_Loops);
			m_pAudioModule->SetMusicVolume(pComponent.m_Volume);
			pComponent.m_CurrentChannel = 0;
			return;
		}

		GScene* pScene = m_pRegistry->GetUserData<GScene>();
		AudioChannelUData udata;
		udata.m_Type = AudioChannelUDataType::Entity;
		udata.sceneID() = pScene->GetUUID();
		udata.entityID() = pScene->GetEntityUUID(entity);

		if(pComponent.m_Enable3D)
			pComponent.m_CurrentChannel = m_pAudioModule->PlayWithEffects(pAudio, pComponent.m_Loops, std::move(udata), FinishedCallback);
		else
			pComponent.m_CurrentChannel = m_pAudioModule->Play(pAudio, pComponent.m_Loops, std::move(udata), FinishedCallback);
		m_pAudioModule->SetVolume(pComponent.m_CurrentChannel, pComponent.m_Volume);
	}

	void AudioSourceManager::Stop(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;
		if (pComponent.m_AsMusic)
		{
			m_pAudioModule->StopMusic();
			pComponent.m_CurrentChannel = -1;
			return;
		}
		m_pAudioModule->Stop(pComponent.m_CurrentChannel);
		pComponent.m_CurrentChannel = -1;
	}

	void AudioSourceManager::Pause(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;
		if (pComponent.m_AsMusic)
		{
			m_pAudioModule->PauseMusic();
			return;
		}
		m_pAudioModule->Pause(pComponent.m_CurrentChannel);
	}

	void AudioSourceManager::Resume(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;
		if (pComponent.m_AsMusic)
		{
			m_pAudioModule->ResumeMusic();
			return;
		}
		m_pAudioModule->Resume(pComponent.m_CurrentChannel);
	}

	void AudioSourceManager::UpdateVolume(Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;
		if (pComponent.m_AsMusic)
		{
			m_pAudioModule->SetMusicVolume(pComponent.m_Volume);
			return;
		}
		m_pAudioModule->SetVolume(pComponent.m_CurrentChannel, pComponent.m_Volume);
	}

	void AudioSourceManager::GetReferencesImpl(std::vector<UUID>& references) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const AudioSource& audioSource = GetAt(i);
			const UUID audio = audioSource.m_Audio.GetUUID();
			if (audio) references.push_back(audio);
		}
	}

	void AudioSourceManager::OnDeserialize(Utils::BinaryStream&)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const AudioSource& audio = GetAt(i);
			audio.m_Audio.ManualRegisterReference();
		}
	}

	void AudioSourceManager::OnInitialize()
	{
		Bind(DoOnRemove, &AudioSourceManager::OnRemoveImpl);
		Bind(DoStart, &AudioSourceManager::OnStartImpl);
		Bind(DoStop, &AudioSourceManager::OnStopImpl);
		Bind(DoValidate, &AudioSourceManager::OnValidateImpl);
		Bind(DoUpdate, &AudioSourceManager::OnUpdateImpl);
		Bind(DoGetReferences, &AudioSourceManager::GetReferencesImpl);
	}

	AudioListenerManager::AudioListenerManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity), m_pAudioModule(nullptr)
	{
	}

	AudioListenerManager::~AudioListenerManager()
	{
	}

	void AudioListenerManager::OnStartImpl(Utils::ECS::EntityID entity, AudioListener& pComponent)
	{
		auto& callback = m_pAudioModule->OnListenerStart;
		if (callback) callback(m_pRegistry, entity, pComponent);
	}

	void AudioListenerManager::OnStopImpl(Utils::ECS::EntityID entity, AudioListener& pComponent)
	{
		auto& callback = m_pAudioModule->OnListenerStop;
		if (callback) callback(m_pRegistry, entity, pComponent);
	}

	void AudioListenerManager::OnUpdateImpl(Utils::ECS::EntityID entity, AudioListener& pComponent, float)
	{
		if (!pComponent.m_Enable) return;
		Transform& transform = m_pRegistry->GetComponent<Transform>(entity);
		m_pAudioModule->ListenerTransform() = transform.MatTransform;
		auto& callback = m_pAudioModule->OnListenerUpdate;
		if (callback) callback(m_pRegistry, entity, pComponent);
	}

	void AudioListenerManager::OnInitialize()
	{
		Bind(DoStart, &AudioListenerManager::OnStartImpl);
		Bind(DoStop, &AudioListenerManager::OnStopImpl);
		Bind(DoUpdate, &AudioListenerManager::OnUpdateImpl);
	}
}
