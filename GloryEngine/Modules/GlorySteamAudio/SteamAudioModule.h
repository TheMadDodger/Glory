#pragma once
#include <Module.h>

#include <phonon/phonon.h>

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
    class SteamAudioModule : public Module
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

		/** @bried Rebuild the @ref IPLScene from loaded @ref AudioScene datas */
		GLORY_API void RebuildAudioSimulationScene();

	protected:
		virtual void Initialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

	private:
		void ProcessEffects(AudioChannel& channel, void* stream, int len);

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
		std::vector<IPLBinauralEffect> m_BinauralEffects;
		std::vector<IPLDirectEffect> m_DirectEffects;
		std::vector<IPLAmbisonicsEncodeEffect> m_AmbiSonicsEffects;
		std::vector<IPLAmbisonicsDecodeEffect> m_AmbiSonicsDecodeEffects;
		std::vector<std::vector<float>> m_TemporaryBuffers;

		std::vector<AudioScene> m_AudioScenes;
    };
}
