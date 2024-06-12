#pragma once
#include <Module.h>

namespace Glory
{
	class AudioData;

    class AudioModule : public Module
    {
	public:
		AudioModule();
		virtual ~AudioModule();

		const std::type_info& GetBaseModuleType() override;

		virtual void Play(AudioData* pAudio, int loops=0) = 0;
		virtual void Stop() = 0;
		virtual void PlayMusic(AudioData* pAudio, int loops=0) = 0;

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() = 0;
    };
}
