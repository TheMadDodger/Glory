#pragma once
#include <AudioModule.h>

#include <SDL2/SDL_mixer.h>

namespace Glory
{
    class SDLAudioModule : public AudioModule
    {
	public:
		SDLAudioModule();
		virtual ~SDLAudioModule();

		const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

		GLORY_API int Play(AudioData* pAudio, void* udata=nullptr, size_t udataSize=0, int loops=0, std::function<void(Engine*, const AudioChannel&)> finishedCallback=NULL) override;
		GLORY_API void Stop(int channel) override;
		GLORY_API void StopMusic() override;
		GLORY_API void StopAll() override;
		GLORY_API void PlayMusic(AudioData* pAudio, int loops=0) override;
		GLORY_API bool IsPlaying(int channel) override;
		GLORY_API bool IsMusicPlaying() override;

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		std::map<UUID, Mix_Chunk*> m_Chunks;
		std::map<UUID, Mix_Music*> m_Music;
    };
}
