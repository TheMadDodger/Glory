#include "AudioSourceSystem.h"
#include "AudioComponents.h"
#include "AudioModule.h"

#include <EntityRegistry.h>
#include <GScene.h>
#include <Engine.h>
#include <SceneManager.h>

namespace Glory
{
	void AudioSourceSystem::OnRemove(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		Stop(pRegistry, entity, pComponent);
	}

	void AudioSourceSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (!pComponent.m_AutoPlay) return;
		Play(pRegistry, entity, pComponent);
	}

	void AudioSourceSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
	}

	void AudioSourceSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
	}

	void FinishedCallback(Engine* pEngine, const AudioChannel& channel)
	{
		const uint64_t* ids = reinterpret_cast<const uint64_t*>(channel.m_UserData);
		GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(UUID(ids[0]));
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(ids[1]);
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
			pComponent.m_CurrentChannel = 0;
			return;
		}

		uint64_t ids[2];
		ids[0] = pScene->GetUUID();
		ids[1] = pScene->GetEntityUUID(entity);
		pComponent.m_CurrentChannel = pAudioModule->Play(pAudio, ids, sizeof(uint64_t)*2, pComponent.m_Loops, FinishedCallback);
	}

	void AudioSourceSystem::Stop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent)
	{
		if (pComponent.m_CurrentChannel == -1) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		AudioModule* pAudioModule = pEngine->GetOptionalModule<AudioModule>();

		AudioData* pAudio = pComponent.m_Audio.Get(&pEngine->GetAssetManager());
		if (!pAudio) return;

		if (pComponent.m_AsMusic)
		{
			pAudioModule->StopMusic();
			pComponent.m_CurrentChannel = -1;
			return;
		}
		pAudioModule->Stop(pComponent.m_CurrentChannel);
		pComponent.m_CurrentChannel = -1;
	}
}
