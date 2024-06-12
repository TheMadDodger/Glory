#include "SDLAudioModule.h"

#include <Engine.h>
#include <SceneManager.h>
#include <AudioData.h>
#include <Debug.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(SDLAudioModule);

	SDLAudioModule::SDLAudioModule()
	{
	}
	SDLAudioModule::~SDLAudioModule() {}

	const std::type_info& SDLAudioModule::GetModuleType()
	{
		return typeid(SDLAudioModule);
	}

	void SDLAudioModule::Play(AudioData* pAudio, int loops)
	{
		auto itor = m_Chunks.find(pAudio->GetUUID());
		if (itor == m_Chunks.end())
		{
			SDL_RWops* pData = SDL_RWFromConstMem(pAudio->Data(), pAudio->Size());
			Mix_Chunk* chunk = Mix_LoadWAV_RW(pData, 0);
			if (!chunk)
			{
				m_pEngine->GetDebug().LogError("Failed to load audio chunk");
			}
			itor = m_Chunks.emplace(pAudio->GetUUID(), chunk).first;
		}

		Mix_PlayChannel(-1, itor->second, loops);
	}

	void SDLAudioModule::Stop()
	{
		Mix_HaltChannel(-1);
	}

	void SDLAudioModule::PlayMusic(AudioData* pAudio, int loops)
	{
		auto itor = m_Music.find(pAudio->GetUUID());
		if (itor == m_Music.end())
		{
			SDL_RWops* pData = SDL_RWFromConstMem(pAudio->Data(), pAudio->Size());
			Mix_Music* music = Mix_LoadMUS_RW(pData, 0);
			if (!music)
			{
				m_pEngine->GetDebug().LogError("Failed to load audio music");
			}
			itor = m_Music.emplace(pAudio->GetUUID(), music).first;
		}

		Mix_PlayMusic(itor->second, loops);
	}

	void SDLAudioModule::Initialize()
	{
		AudioModule::Initialize();

		if (Mix_Init(0) != 0)
		{
			m_pEngine->GetDebug().LogError(Mix_GetError());
			m_pEngine->GetDebug().LogFatalError("Could not initialize SDL_mixer");
			return;
		}

		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) != 0)
		{
			m_pEngine->GetDebug().LogFatalError("Could not initialize SDL_mixer");
			return;
		}
	}

	void SDLAudioModule::Cleanup()
	{
		for (auto itor = m_Chunks.begin(); itor != m_Chunks.end(); ++itor)
		{
			Mix_FreeChunk(itor->second);
		}

		Mix_CloseAudio();
		Mix_Quit();
	}
}