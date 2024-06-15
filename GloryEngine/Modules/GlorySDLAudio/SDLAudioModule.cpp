#include "SDLAudioModule.h"

#include <Engine.h>
#include <SceneManager.h>
#include <AudioData.h>
#include <Debug.h>

namespace Glory
{
	std::vector<AudioChannel> Channels;
	MusicChannel Music;

	GLORY_MODULE_VERSION_CPP(SDLAudioModule);

	SDLAudioModule::SDLAudioModule() { }
	SDLAudioModule::~SDLAudioModule() {}

	const std::type_info& SDLAudioModule::GetModuleType()
	{
		return typeid(SDLAudioModule);
	}

	int SDLAudioModule::Play(AudioData* pAudio, void* udata, size_t udataSize, int loops)
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

		for (size_t i = 0; i < Channels.size(); ++i)
		{
			if (Channels[i].m_CurrentChunk) continue;
			Channels[i].m_CurrentChunk = itor->second;
			if (udata)
				std::memcpy(Channels[i].m_UserData, udata, udataSize);
			if (Mix_PlayChannel(-1, itor->second, loops) == -1)
			{
				m_pEngine->GetDebug().LogError("Failed to play audio.");
				m_pEngine->GetDebug().LogError(Mix_GetError());
				m_pEngine->GetDebug().LogError(Mix_GetError());
				m_pEngine->GetDebug().LogError(Mix_GetError());
				m_pEngine->GetDebug().LogError(Mix_GetError());
				return -1;
			}
#if _DEBUG
			m_pEngine->GetDebug().LogNotice("Used channel " + std::to_string(i) + " to play sound");
#endif // DEBUG
			return int(i);
		}
		
		const size_t oldChannels = Channels.size();
		const size_t newChannels = oldChannels*2.0f;

		const size_t allocated = Mix_AllocateChannels(newChannels);
#if _DEBUG
		m_pEngine->GetDebug().LogNotice("Allocated " + std::to_string(oldChannels) + " new mixing channels for a total of " + std::to_string(allocated));
#endif // DEBUG

		if (allocated != newChannels)
		{
			m_pEngine->GetDebug().LogError(Mix_GetError());
			m_pEngine->GetDebug().LogFatalError("Failed to allocate mixing channels");
			return -1;
		}
		Channels.resize(newChannels);

		for (size_t i = oldChannels; i < Channels.size(); ++i)
		{
			Channels[i].m_Index = i;
		}

		m_pEngine->GetDebug().LogWarning("Allocated more mixing channels because all channels are busy.");
		m_pEngine->GetDebug().LogWarning("Try increasing the mixing channels in the SDL Audio module settings.");

		if (Mix_PlayChannel(int(oldChannels), itor->second, loops) == -1)
		{
			m_pEngine->GetDebug().LogError("Failed to play audio.");
			m_pEngine->GetDebug().LogError(Mix_GetError());
			return -1;
		}
#if _DEBUG
		m_pEngine->GetDebug().LogNotice("Used channel " + std::to_string(oldChannels) + " to play sound");
#endif // DEBUG
		return int(oldChannels);
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

		Music.m_CurrentMusic = itor->second;
		Mix_PlayMusic(itor->second, loops);
	}

	void ChannelFinishedCallback(int channel)
	{
		Channels[size_t(channel)].m_CurrentChunk = NULL;
	}

	void MusicFinishedCallback()
	{
		Music.m_CurrentMusic = NULL;
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

		const uint32_t channels = Settings().Value<unsigned int>("Mixing Channels");
		const uint32_t frequency = Settings().Value<unsigned int>("Frequency");
		const uint32_t chunksize = Settings().Value<unsigned int>("Chunksize");

		if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 8, chunksize) != 0)
		{
			m_pEngine->GetDebug().LogError(Mix_GetError());
			m_pEngine->GetDebug().LogFatalError("Could not initialize SDL_mixer");
			return;
		}

		const size_t allocated = Mix_AllocateChannels(channels);
#if _DEBUG
		m_pEngine->GetDebug().LogNotice("Allocated " + std::to_string(allocated) + " mixing channels");
#endif // DEBUG
		if (allocated != channels)
		{
			m_pEngine->GetDebug().LogError(Mix_GetError());
			m_pEngine->GetDebug().LogFatalError("Failed to allocate mixing channels");
			return;
		}
		Channels.resize(channels);

		for (size_t i = 0; i < Channels.size(); ++i)
		{
			Channels[i].m_Index = i;
		}

		Mix_ChannelFinished(ChannelFinishedCallback);
		Mix_HookMusicFinished(MusicFinishedCallback);
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

	void SDLAudioModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterValue<unsigned int>("Mixing Channels", 64);
		settings.RegisterValue<unsigned int>("Frequency", 22050);
		settings.RegisterValue<unsigned int>("Chunksize", 4096);
	}
}