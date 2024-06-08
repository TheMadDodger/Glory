#include "AudioEmitterSystem.h"
#include "AudioComponents.h"

#include <EntityRegistry.h>
#include <GScene.h>
#include <Engine.h>
#include <SceneManager.h>
#include <SDLAudioModule.h>

namespace Glory
{
	void AudioEmitterSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioEmitter& pComponent)
	{
		if (!pComponent.m_AutoPlay) return;
		Play(pRegistry, entity, pComponent);
	}

	void AudioEmitterSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioEmitter& pComponent)
	{
	}

	void AudioEmitterSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioEmitter& pComponent)
	{
	}

	void AudioEmitterSystem::Play(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioEmitter& pComponent)
	{
		if (!pComponent.m_Audio.AssetUUID()) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		SDLAudioModule* pAudioModule = pEngine->GetOptionalModule<SDLAudioModule>();

		AudioData* pAudio = pComponent.m_Audio.Get(&pEngine->GetAssetManager());
		if (!pAudio) return;

		if (pComponent.m_AsMusic)
		{
			pAudioModule->PlayMusic(pAudio, pComponent.m_Loops);
			return;
		}
		pAudioModule->Play(pAudio, pComponent.m_Loops);
	}
}
