#include "AudioCSAPI.h"
#include "GloryMonoScipting.h"

#include <Debug.h>
#include <AssetManager.h>
#include <AudioModule.h>
#include <AudioData.h>

namespace Glory
{
	Engine* AudioManager_EngineInstance = nullptr;

	float AudioManager_GetMasterVolume()
	{
		AudioModule* pAudioModule = AudioManager_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			AudioManager_EngineInstance->GetDebug().LogError("AudioManager_GetMasterVolume > No audio module was loaded to play audio.");
			return 0.0f;
		}
		return pAudioModule->MasterVolume();
	}

	void AudioManager_SetMasterVolume(float volume)
	{
		AudioModule* pAudioModule = AudioManager_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			AudioManager_EngineInstance->GetDebug().LogError("AudioManager_SetMasterVolume > No audio module was loaded to play audio.");
			return;
		}
		pAudioModule->SetMasterVolume(volume);
	}

	float AudioManager_GetMusicVolume()
	{
		AudioModule* pAudioModule = AudioManager_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			AudioManager_EngineInstance->GetDebug().LogError("AudioManager_GetMusicVolume > No audio module was loaded to play audio.");
			return 0.0f;
		}
		return pAudioModule->MusicVolume();
	}

	void AudioManager_SetMusicVolume(float volume)
	{
		AudioModule* pAudioModule = AudioManager_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			AudioManager_EngineInstance->GetDebug().LogError("AudioManager_GetMusicVolume > No audio module was loaded to play audio.");
			return;
		}
		return pAudioModule->SetMusicVolume(volume);
	}

	void AudioManager_PlayMusic(uint64_t audioId)
	{
		AudioModule* pAudioModule = AudioManager_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			AudioManager_EngineInstance->GetDebug().LogError("AudioManager_GetMusicVolume > No audio module was loaded to play audio.");
			return;
		}

		Resource* pResource = AudioManager_EngineInstance->GetAssetManager().FindResource(audioId);
		if (!pResource) return;
		AudioData* pAudio = static_cast<AudioData*>(pResource);
		pAudioModule->PlayMusic(pAudio);
	}

	void AudioCSAPI::GetInternallCalls(std::vector<InternalCall>& internalCalls)
	{
		BIND("GloryEngine.AudioManager::AudioManager_GetMasterVolume", AudioManager_GetMasterVolume);
		BIND("GloryEngine.AudioManager::AudioManager_SetMasterVolume", AudioManager_SetMasterVolume);
		BIND("GloryEngine.AudioManager::AudioManager_GetMusicVolume", AudioManager_GetMusicVolume);
		BIND("GloryEngine.AudioManager::AudioManager_SetMusicVolume", AudioManager_SetMusicVolume);
		BIND("GloryEngine.AudioManager::AudioManager_PlayMusic", AudioManager_PlayMusic);
	}

	void AudioCSAPI::SetEngine(Engine* pEngine)
	{
		AudioManager_EngineInstance = pEngine;
	}
}
