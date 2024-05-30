#include "SteamAudioModule.h"

#include <fstream>
#include <vector>

#include <Engine.h>
#include <Debug.h>

#define STEAM_AUDIO_VERSION_STR TOSTRING(STEAMAUDIO_VERSION_MAJOR.STEAMAUDIO_VERSION_MINOR.STEAMAUDIO_VERSION_PATCH)

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(SteamAudioModule);

	SteamAudioModule::SteamAudioModule()
	{
	}

	SteamAudioModule::~SteamAudioModule() {}

	const std::type_info& SteamAudioModule::GetModuleType()
	{
		return typeid(SteamAudioModule);
	}

	std::vector<float> load_input_audio(const std::string filename)
	{
		std::ifstream file(filename.c_str(), std::ios::binary);

		file.seekg(0, std::ios::end);
		auto filesize = file.tellg();
		auto numsamples = static_cast<int>(filesize / sizeof(float));

		std::vector<float> inputaudio(numsamples);
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(inputaudio.data()), filesize);

		return inputaudio;
	}

	void save_output_audio(const std::string filename, std::vector<float> outputaudio)
	{
		std::ofstream file(filename.c_str(), std::ios::binary);
		file.write(reinterpret_cast<char*>(outputaudio.data()), outputaudio.size() * sizeof(float));
	}

	void SteamAudioModule::Initialize()
	{
		m_pEngine->GetDebug().LogInfo("SteamAudio version " STEAM_AUDIO_VERSION_STR);

		IPLContextSettings contextSettings{};
		contextSettings.version = STEAMAUDIO_VERSION;

		iplContextCreate(&contextSettings, &m_IPLContext);

		IPLHRTFSettings hrtfSettings{};
		hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
		hrtfSettings.volume = 1.0f;

		IPLAudioSettings audioSettings{};
		audioSettings.samplingRate = 44100;
		audioSettings.frameSize = 1024;

		iplHRTFCreate(m_IPLContext, &audioSettings, &hrtfSettings, &m_IPLHrtf);

		IPLBinauralEffectSettings effectSettings{};
		effectSettings.hrtf = m_IPLHrtf;

		iplBinauralEffectCreate(m_IPLContext, &audioSettings, &effectSettings, &m_IPLEffect);

		std::vector<float> inputaudio = load_input_audio("audiosample.raw");

		std::vector<float> outputaudio;

		float* inData[] = { inputaudio.data() };
		IPLAudioBuffer inBuffer{};
		inBuffer.numChannels = 1;
		inBuffer.numSamples = audioSettings.frameSize;
		inBuffer.data = inData;

		IPLAudioBuffer outBuffer{};
		iplAudioBufferAllocate(m_IPLContext, 2, audioSettings.frameSize, &outBuffer);

		std::vector<float> outputaudioframe(2 * audioSettings.frameSize);

		int numframes = inputaudio.size() / audioSettings.frameSize;

		for (int i = 0; i < numframes; ++i)
		{
			// render a frame of spatialized audio and append to the end of outputaudio
			IPLBinauralEffectParams effectParams{};
			effectParams.direction = IPLVector3{ 1.0f, 1.0f, 1.0f };
			effectParams.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
			effectParams.spatialBlend = 1.0f;
			effectParams.hrtf = m_IPLHrtf;
			effectParams.peakDelays = nullptr;

			iplBinauralEffectApply(m_IPLEffect, &effectParams, &inBuffer, &outBuffer);

			iplAudioBufferInterleave(m_IPLContext, &outBuffer, outputaudioframe.data());

			std::copy(std::begin(outputaudioframe), std::end(outputaudioframe), std::back_inserter(outputaudio));

			// advance the input to the next frame
			inData[0] += audioSettings.frameSize;
		}

		iplAudioBufferFree(m_IPLContext, &outBuffer);

		save_output_audio("outputaudio.raw", outputaudio);
	}

	void SteamAudioModule::Cleanup()
	{
		iplBinauralEffectRelease(&m_IPLEffect);
		iplHRTFRelease(&m_IPLHrtf);
		iplContextRelease(&m_IPLContext);
	}
}