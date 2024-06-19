#pragma once
#include <AudioModule.h>

#include <SDL2/SDL_mixer.h>

namespace Glory
{
	/** @brief SDL Audio Module using SDL_mixer */
    class SDLAudioModule : public AudioModule
    {
	public:
		/** @brief Constructor */
		SDLAudioModule();
		/** @brief Destructor */
		virtual ~SDLAudioModule();

		/** @brief AudioModule type */
		const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

		/** @brief Play an audio resource
		 * @param pAudio Audio asset to play
		 * @param udata Pointer to data to store in the channels user data buffer
		 * @param udataSize Size of the user data
		 * @param loops How many times to loop the audio
		 * @param finishedCallback Callback that will be called when the audio finishes playing
		 * @returns The channel that was chosen to play the audio
		 */
		GLORY_API int Play(AudioData* pAudio, int loops=0, AudioChannelUData&& udata={}, std::function<void(Engine*, const AudioChannel&)> finishedCallback=NULL) override;
		GLORY_API int PlayWithEffects(AudioData* pAudio, int loops = 0, AudioChannelUData&& udata={}, std::function<void(Engine*, const AudioChannel&)> finishedCallback = NULL) override;
		GLORY_API AudioChannel& Channel(int channel) override;

		/** @brief Stop a channel from playing
		 * @param channel Channel to stop
		 */
		GLORY_API void Stop(int channel) override;
		/** @brief Stop music */
		GLORY_API void StopMusic() override;
		/** @brief Stop all channels */
		GLORY_API void StopAll() override;
		/** @brief Play an audio asset as music
		 * @param pAudio Audio asset to play
		 * @param loops How many times to loop the music
		 */
		GLORY_API void PlayMusic(AudioData* pAudio, int loops=0) override;
		/** @brief Is a specific channel currently playing
		 * @param channel Channel to check
		 */
		GLORY_API bool IsPlaying(int channel) override;
		/** @brief Is music currently playing */
		GLORY_API bool IsMusicPlaying() override;
		/** @brief Pause the entire sound engine */
		GLORY_API void Pause() override;
		/** @brief Pause a specific channel
		 * @param channel Channel to pause
		 */
		GLORY_API void Pause(int channel) override;
		/** @brief Pause music */
		GLORY_API void PauseMusic() override;
		/** @brief Is a specific channel currently paused
		 * @param channel Channel to check
		 */
		GLORY_API bool IsPaused(int channel) override;
		/** @brief Is the music paused */
		GLORY_API bool IsMusicPaused() override;
		/** @brief Resume the audio engine */
		GLORY_API void Resume() override;
		/** @brief Resume a specific channel
		 * @param channel Channel to resume
		 */
		GLORY_API void Resume(int channel) override;
		/** @brief Resume music */
		GLORY_API void ResumeMusic() override;
		/** @brief Set the volume of a specific channel
		 * @param channel Channel to resume
		 * @param volume Volume to set
		 */
		GLORY_API void SetVolume(int channel, float volume) override;
		/** @brief Set the volume of the music
		 * @param volume Volume to set
		 */
		GLORY_API void SetMusicVolume(float volume) override;
		/** @brief Set the master volume of the audio engine
		 * @param volume Volume to set
		 */
		GLORY_API void SetMasterVolume(float volume) override;
		/** @brief Get volume of a channel
		 * @param channel Channel to get the volume from
		 */
		GLORY_API float Volume(int channel) override;
		/** @brief Get volume of the music */
		GLORY_API float MusicVolume() override;
		/** @brief Get master volume of the audio engine */
		GLORY_API float MasterVolume() override;

		GLORY_API uint8_t* GetChunkData(void* chunk) override;

		GLORY_API uint32_t SamplingRate() { return uint32_t(m_Frequency); }
		GLORY_API uint32_t Channels() override { return uint32_t(m_Channels); }
		GLORY_API uint32_t MixingChannels() override;
		GLORY_API glm::mat4& ListenerTransform() override { return m_ListerenTransform; }

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() override;

	private:
		std::map<UUID, Mix_Chunk*> m_Chunks;
		std::map<UUID, Mix_Music*> m_Music;

		int m_Frequency;
		uint16_t m_Format;
		int m_Channels;

		glm::mat4 m_ListerenTransform;
    };
}
