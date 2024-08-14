#pragma once
#include <Audio3DModule.h>

#include <phonon/phonon.h>

#include <SoundMaterialData.h>

#include <EntityID.h>

namespace Glory::Utils::ECS
{
	class EntityRegistry;
}

namespace Glory
{
	struct AudioChannel;
	struct AudioSource;
	class AudioModule;
	class AudioScene;

	/** @brief SteamAudio module */
    class SteamAudioModule : public Audio3DModule
    {
	public:
		/** @brief Constructor */
		SteamAudioModule();
		/** @brief Destructor */
		virtual ~SteamAudioModule();

		/** @brief Module type - typeid(SteamAudioModule) */
		virtual const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

		GLORY_API IPLContext GetContext();

		/**
		 * @brief Add an audio scene for audio simulation
		 * @param audioScene The @ref AudioScene to add
		 *
		 * @note You must call @ref RebuildAudioSimulationScene() before the changes take effect
		 */
		GLORY_API void AddAudioScene(AudioScene&& audioScene);
		/**
		 * @brief Remove all loaded audio scenes from the simulation
		 *
		 * @note You must call @ref RebuildAudioSimulationScene() before the changes take effect
		 */
		GLORY_API void RemoveAllAudioScenes();

		/** @brief Rebuild the @ref IPLScene from loaded @ref AudioScene datas */
		GLORY_API void RebuildAudioSimulationScene();

		/** @brief Get the default sound material */
		GLORY_API const SoundMaterial& DefaultMaterial() const;
		/** @overload */
		GLORY_API SoundMaterial& DefaultMaterial();

		/**
		 * @brief Check if a feature is supported
		 * @param feature Hash of the feature to check
		 *
		 * Supported features:
		 * - Occlusion
		 * - Transmission
		 * - Absorption
		 * - Reflection
		 */
		GLORY_API bool HasFeature(uint32_t feature) const override;

	protected:
		virtual void Initialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

	private:
		/**
		 * @brief Process spatialization and simulation effects
		 * @param channel Audio channel currently requesting processing
		 * @param stream Current audio stream
		 * @param len Length of the current stream
		 */
		void ProcessEffects(AudioChannel& channel, void* stream, int len);

		/**
		 * @brief Spatialize a stream with a binaural effect
		 * @param channel Audio channel currently requesting processing
		 * @param dir Direction of the audio source relative to the listener
		 * @param spatialBlend How much to blend the spatialization with the original audio
		 * @param stream Current audio stream
		 */
		void SpatializeBinaural(AudioChannel& channel, const glm::vec3& dir, float spatialBlend, void* stream);

		/**
		 * @brief Spatialize a stream with an ambisonics effect
		 * @param channel Audio channel currently requesting processing
		 * @param listenPos Position of the listener
		 * @param dir Direction of the audio source relative to the listener
		 * @param stream Current audio stream
		 */
		void SpatializeAmbisonics(AudioChannel& channel, const glm::vec3& listenPos, const glm::vec3& dir, int order, void* stream);

		/**
		 * @brief Calculate attenuation factor for an audio source based on distance to the listener
		 * @param listenPos Position of the listener
		 * @param sourcePos Position of the source
		 * @param stream Current audio stream
		 */
		float Attenuate(const glm::vec3& listenPos, const glm::vec3& sourcePos) const;

		/**
		 * @brief Prepare all effects for the specified number of mixing channels
		 * @param channels Number of mixing channels to prepare for
		 */
		void AllocateChannels(size_t channels);

		/**
		 * @brief Add a new source to the simulation
		 * @param pRegistry Registry that owns the entity
		 * @param entity ID of the entity
		 * @param source The @ref AudioSource component
		 */
		void AddSource(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& source);
		/**
		 * @brief Update a source its position in the simulation
		 * @param pRegistry Registry that owns the entity
		 * @param entity ID of the entity
		 * @param source The @ref AudioSource component
		 */
		void UpdateSource(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& source);
		/**
		 * @brief Remove a source from the simulation
		 * @param pRegistry Registry that owns the entity
		 * @param entity ID of the entity
		 * @param source The @ref AudioSource component
		 */
		void RemoveSource(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& source);

	private:
		IPLContext m_IPLContext = nullptr;
		IPLHRTF m_IPLHrtf = nullptr;
		IPLScene m_Scene = nullptr;
		IPLSimulator m_Simulator = nullptr;
		AudioModule* m_pAudioModule = nullptr;
		std::vector<IPLSource> m_Sources;
		std::vector<UUID> m_SourceEntities;

		std::vector<IPLAudioBuffer> m_InBuffers;
		std::vector<IPLAudioBuffer> m_OutBuffers;
		std::vector<IPLAudioBuffer> m_AmbisonicsBuffers;
		std::vector<int> m_AmbiSonicsOrders;
		std::vector<IPLBinauralEffect> m_BinauralEffects;
		std::vector<IPLDirectEffect> m_DirectEffects;
		std::vector<IPLAmbisonicsEncodeEffect> m_AmbiSonicsEffects;
		std::vector<IPLAmbisonicsDecodeEffect> m_AmbiSonicsDecodeEffects;
		std::vector<std::vector<float>> m_TemporaryBuffers;

		std::vector<AudioScene> m_AudioScenes;

		SoundMaterial m_DefaultSoundMaterial;
    };
}
