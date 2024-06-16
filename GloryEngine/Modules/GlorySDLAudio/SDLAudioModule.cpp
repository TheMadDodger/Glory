#include "SDLAudioModule.h"

#include <Engine.h>
#include <SceneManager.h>
#include <AudioData.h>
#include <Debug.h>

#define MIX_MAX_VOLUME_FLOAT MIX_MAX_VOLUME##.0f

namespace Glory
{
	std::vector<AudioChannel> Channels;
	MusicChannel Music;

	Engine* Audio_EngineInstance = nullptr;

	GLORY_MODULE_VERSION_CPP(SDLAudioModule);

	SDLAudioModule::SDLAudioModule() { }
	SDLAudioModule::~SDLAudioModule() {}

	const std::type_info& SDLAudioModule::GetModuleType()
	{
		return typeid(SDLAudioModule);
	}

	int SDLAudioModule::Play(AudioData* pAudio, void* udata, size_t udataSize, int loops, std::function<void(Engine*, const AudioChannel&)> finishedCallback)
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
			Channels[i].m_FinishedCallback = finishedCallback;
			if (Mix_PlayChannel(-1, itor->second, loops) == -1)
			{
				m_pEngine->GetDebug().LogError("Failed to play audio.");
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

		Channels[oldChannels].m_FinishedCallback = finishedCallback;
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

	void SDLAudioModule::Stop(int channel)
	{
		/* Will trigger the finished callback which will cleanup the channel */
		if (Mix_HaltChannel(channel) != 0)
		{
			m_pEngine->GetDebug().LogError("Failed to stop audio.");
			m_pEngine->GetDebug().LogError(Mix_GetError());
		}
	}

	void SDLAudioModule::StopMusic()
	{
		Mix_HaltMusic();
	}

	void SDLAudioModule::StopAll()
	{
		if (Mix_HaltChannel(-1) != 0)
		{
			m_pEngine->GetDebug().LogError("Failed to stop all audio.");
			m_pEngine->GetDebug().LogError(Mix_GetError());
		}
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

	bool SDLAudioModule::IsPlaying(int channel)
	{
		return Mix_Playing(channel);
	}

	bool SDLAudioModule::IsMusicPlaying()
	{
		return Mix_PlayingMusic();
	}

	void SDLAudioModule::Pause()
	{
		Mix_PauseAudio(1);
	}

	void SDLAudioModule::Pause(int channel)
	{
		Mix_Pause(channel);
	}

	void SDLAudioModule::PauseMusic()
	{
		Mix_PauseMusic();
	}

	bool SDLAudioModule::IsPaused(int channel)
	{
		return Mix_Paused(channel);
	}

	bool SDLAudioModule::IsMusicPaused()
	{
		return Mix_PausedMusic();
	}

	void SDLAudioModule::Resume()
	{
		Mix_PauseAudio(0);
	}

	void SDLAudioModule::Resume(int channel)
	{
		Mix_Resume(channel);
	}

	void SDLAudioModule::ResumeMusic()
	{
		Mix_ResumeMusic();
	}

	void SDLAudioModule::SetVolume(int channel, float volume)
	{
		Mix_Volume(channel, int(MIX_MAX_VOLUME*volume));
	}

	void SDLAudioModule::SetMusicVolume(float volume)
	{
		Mix_VolumeMusic(int(MIX_MAX_VOLUME*volume));
	}

	void SDLAudioModule::SetMasterVolume(float volume)
	{
		Mix_MasterVolume(int(MIX_MAX_VOLUME * volume));
	}

	float SDLAudioModule::Volume(int channel)
	{
		return Mix_Volume(channel, -1)/MIX_MAX_VOLUME_FLOAT;
	}

	float SDLAudioModule::MusicVolume()
	{
		return Mix_VolumeMusic(-1)/MIX_MAX_VOLUME_FLOAT;
	}

	float SDLAudioModule::MasterVolume()
	{
		return Mix_MasterVolume(-1)/MIX_MAX_VOLUME_FLOAT;
	}

	void ChannelFinishedCallback(int channel)
	{
		AudioChannel& channelData = Channels[size_t(channel)];
		if (channelData.m_FinishedCallback)
			channelData.m_FinishedCallback(Audio_EngineInstance, channelData);
		channelData.m_CurrentChunk = NULL;
		channelData.m_FinishedCallback = NULL;

#if _DEBUG
		Audio_EngineInstance->GetDebug().LogNotice("Channel " + std::to_string(channel) + " has finished playing");
#endif // DEBUG
	}

	void MusicFinishedCallback()
	{
		Music.m_CurrentMusic = NULL;
	}

	void SDLAudioModule::Initialize()
	{
		Audio_EngineInstance = m_pEngine;

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