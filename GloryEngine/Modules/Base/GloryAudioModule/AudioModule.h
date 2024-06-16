#pragma once
#include <Module.h>

#include <functional>

namespace Glory
{
	class AudioData;
	class AudioSourceSystem;

	struct AudioChannel
	{
		size_t m_Index = 0;
		void* m_CurrentChunk = NULL;
		char m_UserData[128] = "\0";
		std::function<void(Engine*, const AudioChannel&)> m_FinishedCallback = NULL;
	};

	struct MusicChannel
	{
		void* m_CurrentMusic = NULL;
	};

    class AudioModule : public Module
    {
	public:
		AudioModule();
		virtual ~AudioModule();

		const std::type_info& GetBaseModuleType() override;

		virtual int Play(AudioData* pAudio, void* udata=nullptr, size_t udataSize=0, int loops=0, std::function<void(Engine*, const AudioChannel&)> finishedCallback=NULL) = 0;
		virtual void Stop(int channel) = 0;
		virtual void StopMusic() = 0;
		virtual void StopAll() = 0;
		virtual void PlayMusic(AudioData* pAudio, int loops=0) = 0;
		virtual bool IsPlaying(int channel) = 0;
		virtual bool IsMusicPlaying() = 0;
		virtual void Pause() = 0;
		virtual void Pause(int channel) = 0;
		virtual void PauseMusic() = 0;
		virtual bool IsPaused(int channel) = 0;
		virtual bool IsMusicPaused() = 0;
		virtual void Resume() = 0;
		virtual void Resume(int channel) = 0;
		virtual void ResumeMusic() = 0;
		virtual void SetVolume(int channel, float volume) = 0;
		virtual void SetMusicVolume(float volume) = 0;
		virtual void SetMasterVolume(float volume) = 0;
		virtual float Volume(int channel) = 0;
		virtual float MusicVolume() = 0;
		virtual float MasterVolume() = 0;

		//virtual AudioSourceSystem* AudioSourceManager() = 0;

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() = 0;
    };
}
