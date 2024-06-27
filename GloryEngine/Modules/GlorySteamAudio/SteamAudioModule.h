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

    class SteamAudioModule : public Module
    {
	public:
		SteamAudioModule();
		virtual ~SteamAudioModule();

		virtual const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

		GLORY_API IPLContext GetContext();

		GLORY_API void SetScene(IPLScene scene);
		GLORY_API IPLScene GetScene();

	protected:
		virtual void Initialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

	private:
		void ProcessEffects(AudioChannel& channel, void* stream, int len);

		void AllocateChannels(size_t channels);

		void AddSource(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& source);
		void RemoveSource(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& source);

	private:
		IPLContext m_IPLContext = nullptr;
		IPLHRTF m_IPLHrtf = nullptr;
		IPLScene m_Scene = nullptr;
		IPLSimulator m_Simulator = nullptr;
		AudioModule* m_pAudioModule = nullptr;
		std::vector<IPLSource> m_Sources;

		std::vector<IPLAudioBuffer> m_InBuffers;
		std::vector<IPLAudioBuffer> m_OutBuffers;
		std::vector<IPLAudioBuffer> m_AmbisonicsBuffers;
		std::vector<IPLBinauralEffect> m_BinauralEffects;
		std::vector<IPLDirectEffect> m_DirectEffects;
		std::vector<IPLAmbisonicsEncodeEffect> m_AmbiSonicsEffects;
		std::vector<IPLAmbisonicsDecodeEffect> m_AmbiSonicsDecodeEffects;
		std::vector<std::vector<float>> m_TemporaryBuffers;
    };
}
