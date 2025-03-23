#include "AudioSourceSystem.h"
#include "AudioComponents.h"
#include "AudioModule.h"

#include <EntityRegistry.h>
#include <GScene.h>
#include <Engine.h>
#include <SceneManager.h>
#include <Components.h>

namespace Glory
{
	void AudioSourceSystem::OnRemove(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		Stop(pRegistry, entity, pComponent);
	}

	void AudioSourceSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		auto& callback = pAudioModule->SourceSystem().OnSourceStart;
		if (callback) callback(pRegistry, entity, pComponent);

		if (!pComponent.m_AutoPlay) return;
		Play(pRegistry, entity, pComponent);
	}

	void AudioSourceSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		UpdateVolume(pRegistry, entity, pComponent);
	}

	void AudioSourceSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		auto& callback = pAudioModule->SourceSystem().OnSourceStop;
		if (callback) callback(pRegistry, entity, pComponent);

		Stop(pRegistry, entity, pComponent);
	}

	void AudioSourceSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		auto& callback = pAudioModule->SourceSystem().OnSourceUpdate;
		if (callback) callback(pRegistry, entity, pComponent);
	}

	void FinishedCallback(Engine* pEngine, const AudioChannel& channel)
	{
		if (channel.m_UserData.m_Type != AudioChannelUDataType::Entity) return;
		GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(channel.m_UserData.sceneID());
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(channel.m_UserData.entityID());
		if (!entity.IsValid()) return;
		if (!entity.HasComponent<AudioSource>()) return;
		entity.GetComponent<AudioSource>().m_CurrentChannel = -1;
	}

	void AudioSourceSystem::Play(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (!pComponent.m_Audio.AssetUUID()) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		AudioData* pAudio = pComponent.m_Audio.Get(&pEngine->GetAssetManager());
		if (!pAudio) return;

		if (pComponent.m_AsMusic)
		{
			pAudioModule->PlayMusic(pAudio, pComponent.m_Loops);
			pAudioModule->SetMusicVolume(pComponent.m_Volume);
			pComponent.m_CurrentChannel = 0;
			return;
		}

		AudioChannelUData udata;
		udata.m_Type = AudioChannelUDataType::Entity;
		udata.sceneID() = pScene->GetUUID();
		udata.entityID() = pScene->GetEntityUUID(entity);

		if(pComponent.m_Enable3D)
			pComponent.m_CurrentChannel = pAudioModule->PlayWithEffects(pAudio, pComponent.m_Loops, std::move(udata), FinishedCallback);
		else
			pComponent.m_CurrentChannel = pAudioModule->Play(pAudio, pComponent.m_Loops, std::move(udata), FinishedCallback);
		pAudioModule->SetVolume(pComponent.m_CurrentChannel, pComponent.m_Volume);
	}

	void AudioSourceSystem::Stop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		if (pComponent.m_AsMusic)
		{
			pAudioModule->StopMusic();
			pComponent.m_CurrentChannel = -1;
			return;
		}
		pAudioModule->Stop(pComponent.m_CurrentChannel);
		pComponent.m_CurrentChannel = -1;
	}

	void AudioSourceSystem::Pause(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		if (pComponent.m_AsMusic)
		{
			pAudioModule->PauseMusic();
			return;
		}
		pAudioModule->Pause(pComponent.m_CurrentChannel);
	}

	void AudioSourceSystem::Resume(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		if (pComponent.m_AsMusic)
		{
			pAudioModule->ResumeMusic();
			return;
		}
		pAudioModule->Resume(pComponent.m_CurrentChannel);
	}

	void AudioSourceSystem::UpdateVolume(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		if (pComponent.m_AsMusic)
		{
			pAudioModule->SetMusicVolume(pComponent.m_Volume);
			return;
		}
		pAudioModule->SetVolume(pComponent.m_CurrentChannel, pComponent.m_Volume);
	}

	void AudioSourceSystem::GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references)
	{
		for (size_t i = 0; i < pTypeView->Size(); ++i)
		{
			const AudioSource* pAudioSource = static_cast<const AudioSource*>(pTypeView->GetComponentAddressFromIndex(i));
			const UUID audio = pAudioSource->m_Audio.AssetUUID();
			if (audio) references.push_back(audio);
		}
	}

	void AudioListenerSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioListener& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		auto& callback = pAudioModule->ListenerSystem().OnListenerStart;
		if (callback) callback(pRegistry, entity, pComponent);
	}

	void AudioListenerSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioListener& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		auto& callback = pAudioModule->ListenerSystem().OnListenerStop;
		if (callback) callback(pRegistry, entity, pComponent);
	}

	void AudioListenerSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioListener& pComponent)
	{
		if (!pComponent.m_Enable) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		Transform& transform = pRegistry->GetComponent<Transform>(entity);

		pAudioModule->ListenerTransform() = transform.MatTransform;

		auto& callback = pAudioModule->ListenerSystem().OnListenerUpdate;
		if (callback) callback(pRegistry, entity, pComponent);
	}
}
