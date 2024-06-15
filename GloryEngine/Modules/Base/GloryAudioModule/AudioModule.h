#pragma once
#include <Module.h>

namespace Glory
{
	class AudioData;
	class AudioSourceSystem;

	struct AudioChannel
	{
		size_t m_Index = 0;
		void* m_CurrentChunk = NULL;
		char m_UserData[128] = "\0";
		//std::function<void(const AudioChannel&)> m_FinishedCallback;
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

		virtual int Play(AudioData* pAudio, void* udata=nullptr, size_t udataSize=0, int loops=0) = 0;
		virtual void Stop() = 0;
		virtual void PlayMusic(AudioData* pAudio, int loops=0) = 0;

		virtual AudioSourceSystem* AudioSourceManager() = 0;

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() = 0;
    };
}
