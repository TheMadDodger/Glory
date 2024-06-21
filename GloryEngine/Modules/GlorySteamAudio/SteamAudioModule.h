#pragma once
#include <Module.h>

#include <phonon/phonon.h>

namespace Glory
{
	struct AudioChannel;
	class AudioModule;

    class SteamAudioModule : public Module
    {
	public:
		SteamAudioModule();
		virtual ~SteamAudioModule();

		virtual const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() override;

	private:
		void ProcessEffects(AudioChannel& channel, void* stream, int len);

		void AllocateChannels(size_t channels);

	private:
		IPLContext m_IPLContext = nullptr;
		IPLHRTF m_IPLHrtf = nullptr;
		AudioModule* m_pAudioModule = nullptr;

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
