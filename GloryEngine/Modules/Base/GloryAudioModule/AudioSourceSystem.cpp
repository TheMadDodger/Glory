#include "AudioSourceSystem.h"
#include "AudioComponents.h"
#include "AudioModule.h"

#include <EntityRegistry.h>
#include <GScene.h>
#include <Engine.h>
#include <SceneManager.h>

namespace Glory
{
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
			return;
		}
		pAudioModule->Play(pAudio, nullptr, 0, pComponent.m_Loops);
	}
}
