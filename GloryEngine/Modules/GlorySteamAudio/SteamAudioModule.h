#pragma once
#include <Module.h>

#include <phonon/phonon.h>

namespace Glory
{
    class SteamAudioModule : public Module
    {
	public:
		SteamAudioModule();
		virtual ~SteamAudioModule();

		virtual const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

	protected: // Internal functions

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() override;

	private:
		IPLContext m_IPLContext = nullptr;
		IPLHRTF m_IPLHrtf = nullptr;
		IPLBinauralEffect m_IPLEffect = nullptr;
    };
}
