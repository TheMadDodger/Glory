#pragma once
#include <Module.h>

#include <SDL2/SDL_mixer.h>

namespace Glory
{
	class AudioData;

    class SDLAudioModule : public Module
    {
	public:
		SDLAudioModule();
		virtual ~SDLAudioModule();

		const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

		GLORY_API void Play(AudioData* pAudio, int loops=0);
		GLORY_API void Stop();
		GLORY_API void PlayMusic(AudioData* pAudio, int loops=0);

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() override;

	private:
		std::map<UUID, Mix_Chunk*> m_Chunks;
		std::map<UUID, Mix_Music*> m_Music;
    };
}
