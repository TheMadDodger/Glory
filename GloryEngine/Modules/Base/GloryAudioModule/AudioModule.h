#pragma once
#include <Module.h>

#include <functional>
#include <glm/glm.hpp>

#include <memory>

namespace Glory
{
	class AudioData;
	class AudioSourceSystem;
	class AudioListenerSystem;

	enum class AudioChannelUDataType
	{
		None = 0,
		Position = 1,
		Entity = 2
	};

	struct AudioChannelUData
	{
		AudioChannelUDataType m_Type = AudioChannelUDataType::None;
		char m_UserData[128] = "\0";

		const glm::vec3& pos() { return *reinterpret_cast<glm::vec3*>(m_UserData); }
		UUID& sceneID() { return *reinterpret_cast<UUID*>(m_UserData); }
		UUID& entityID() { return *reinterpret_cast<UUID*>(&m_UserData[sizeof(UUID)]); }
		const UUID& sceneID() const { return *reinterpret_cast<const UUID*>(m_UserData); }
		const UUID& entityID() const { return *reinterpret_cast<const UUID*>(&m_UserData[sizeof(UUID)]); }
	};

	/** @brief Audio channel */
	struct AudioChannel
	{
		size_t m_Index = 0;
		void* m_CurrentChunk = NULL;
		AudioChannelUData m_UserData;
		std::function<void(Engine*, const AudioChannel&)> m_FinishedCallback = NULL;
	};

	/** @brief Music channel */
	struct MusicChannel
	{
		void* m_CurrentMusic = NULL;
	};

	/** @brief Base class for audio backend and mixing modules */
    class AudioModule : public Module
    {
	public:
		/** @brief Constructor */
		AudioModule();
		/** @brief Destructor */
		virtual ~AudioModule();

		/** @brief AudioModule type */
		const std::type_info& GetBaseModuleType() override;

		/** @brief Play an audio resource
		 * @param pAudio Audio asset to play
		 * @param udata Pointer to data to store in the channels user data buffer
		 * @param udataSize Size of the user data
		 * @param loops How many times to loop the audio
		 * @param finishedCallback Callback that will be called when the audio finishes playing
		 * @returns The channel that was chosen to play the audio
		 */
		virtual int Play(AudioData* pAudio, int loops = 0, AudioChannelUData&& udata={}, std::function<void(Engine*, const AudioChannel&)> finishedCallback = NULL) = 0;
		virtual int PlayWithEffects(AudioData* pAudio, int loops=0, AudioChannelUData&& udata={}, std::function<void(Engine*, const AudioChannel&)> finishedCallback=NULL) = 0;
		virtual AudioChannel& Channel(int channel) = 0;

		/** @brief Stop a channel from playing
		 * @param channel Channel to stop
		 */
		virtual void Stop(int channel) = 0;
		/** @brief Stop music */
		virtual void StopMusic() = 0;
		/** @brief Stop all channels */
		virtual void StopAll() = 0;
		/** @brief Play an audio asset as music
		 * @param pAudio Audio asset to play
		 * @param loops How many times to loop the music
		 */
		virtual void PlayMusic(AudioData* pAudio, int loops=0) = 0;
		/** @brief Is a specific channel currently playing
		 * @param channel Channel to check
		 */
		virtual bool IsPlaying(int channel) = 0;
		/** @brief Is music currently playing */
		virtual bool IsMusicPlaying() = 0;
		/** @brief Pause the entire sound engine */
		virtual void Pause() = 0;
		/** @brief Pause a specific channel
		 * @param channel Channel to pause
		 */
		virtual void Pause(int channel) = 0;
		/** @brief Pause music */
		virtual void PauseMusic() = 0;
		/** @brief Is a specific channel currently paused
		 * @param channel Channel to check
		 */
		virtual bool IsPaused(int channel) = 0;
		/** @brief Is the music paused */
		virtual bool IsMusicPaused() = 0;
		/** @brief Resume the audio engine */
		virtual void Resume() = 0;
		/** @brief Resume a specific channel
		 * @param channel Channel to resume
		 */
		virtual void Resume(int channel) = 0;
		/** @brief Resume music */
		virtual void ResumeMusic() = 0;
		/** @brief Set the volume of a specific channel
		 * @param channel Channel to resume
		 * @param volume Volume to set
		 */
		virtual void SetVolume(int channel, float volume) = 0;
		/** @brief Set the volume of the music
		 * @param volume Volume to set
		 */
		virtual void SetMusicVolume(float volume) = 0;
		/** @brief Set the master volume of the audio engine
		 * @param volume Volume to set
		 */
		virtual void SetMasterVolume(float volume) = 0;
		/** @brief Get volume of a channel
		 * @param channel Channel to get the volume from
		 */
		virtual float Volume(int channel) = 0;
		/** @brief Get volume of the music */
		virtual float MusicVolume() = 0;
		/** @brief Get master volume of the audio engine */
		virtual float MasterVolume() = 0;

		virtual uint8_t* GetChunkData(void* chunk) = 0;

		virtual uint32_t SamplingRate() = 0;
		virtual uint32_t Channels() = 0;
		virtual uint32_t MixingChannels() = 0;
		virtual glm::mat4& ListenerTransform() = 0;

		AudioSourceSystem& SourceSystem();
		AudioListenerSystem& ListenerSystem();

		std::function<void(AudioChannel&, void*, int)> OnEffectCallback;
		std::function<void(size_t)> OnMixingChannelsResized;

		struct SettingNames
		{
			static constexpr char* MixingChannels = "Mixing MixChannels";
			static constexpr char* SamplingRate = "Sampling Rate";
			static constexpr char* Framesize = "Framesize";
		};

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() = 0;
		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		std::unique_ptr<AudioSourceSystem> m_AudioSourceSystem;
		std::unique_ptr<AudioListenerSystem> m_AudioListenerSystem;
    };
}
