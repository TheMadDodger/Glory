#include "SteamAudioModule.h"

#include <fstream>
#include <vector>
#include <glm/gtx/matrix_decompose.hpp>

#include <Engine.h>
#include <Debug.h>
#include <SceneManager.h>
#include <GScene.h>
#include <Entity.h>
#include <Components.h>

#include <AudioModule.h>

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

	void SteamAudioModule::Initialize()
	{
		m_pAudioModule = m_pEngine->GetOptionalModule<AudioModule>();
		if (!m_pAudioModule)
		{
			m_pEngine->GetDebug().LogError("SteamAudio requires an external audio backend and mixing module.");
			return;
		}

		m_pEngine->GetDebug().LogInfo("SteamAudio version " STEAM_AUDIO_VERSION_STR);

		IPLContextSettings contextSettings{};
		contextSettings.version = STEAMAUDIO_VERSION;

		iplContextCreate(&contextSettings, &m_IPLContext);

		IPLHRTFSettings hrtfSettings{};
		hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
		hrtfSettings.volume = 1.0f;

		ModuleSettings& audioModuleSettings = m_pAudioModule->Settings();
		const unsigned int samplingRate = m_pAudioModule->SamplingRate();
		if (samplingRate == 0)
		{
			m_pEngine->GetDebug().LogError("Sampling rate is 0! Make sure your AudioModule is initialized before SteamAudio!");
			return;
		}

		const unsigned int farmeSize = audioModuleSettings.Value<unsigned int>(AudioModule::SettingNames::Framesize);

		IPLAudioSettings audioSettings{};
		audioSettings.samplingRate = samplingRate;
		audioSettings.frameSize = farmeSize;

		iplHRTFCreate(m_IPLContext, &audioSettings, &hrtfSettings, &m_IPLHrtf);

		m_pAudioModule->OnMixingChannelsResized = [this](size_t channels) {
			AllocateChannels(channels);
		};
		AllocateChannels(m_pAudioModule->MixingChannels());

		m_pAudioModule->OnEffectCallback = [this](AudioChannel& channel, void* stream, int len) {
			ProcessEffects(channel, stream, len);
		};
	}

	void SteamAudioModule::Cleanup()
	{
		for (size_t i = 0; i < m_InBuffers.size(); ++i)
		{
			iplAudioBufferFree(m_IPLContext, &m_InBuffers[i]);
			iplAudioBufferFree(m_IPLContext, &m_OutBuffers[i]);
			iplBinauralEffectRelease(&m_BinauralEffects[i]);
		}

		iplHRTFRelease(&m_IPLHrtf);
		iplContextRelease(&m_IPLContext);
	}

	void SteamAudioModule::ProcessEffects(AudioChannel& channel, void* stream, int len)
	{
		glm::vec3 dir{};
		const glm::mat4 listener = m_pAudioModule->ListenerTransform();
		glm::vec3 scale, listenPos, skew;
		glm::vec4 pers;
		glm::quat listenRot;
		glm::decompose(listener, scale, listenRot, listenPos, skew, pers);

		switch (channel.m_UserData.m_Type)
		{
		case AudioChannelUDataType::Entity:
		{
			GScene* pScene = m_pEngine->GetSceneManager()->GetOpenScene(channel.m_UserData.sceneID());
			if (!pScene) return;
			Entity entity = pScene->GetEntityByUUID(channel.m_UserData.entityID());
			if (!entity.IsValid()) return;
			const glm::vec3 pos = entity.GetComponent<Transform>().Position;
			dir = pos - listenPos;
			dir = dir*listenRot;
			break;
		}
		default:
			break;
		}

		ModuleSettings& audioSettings = m_pAudioModule->Settings();
		const unsigned int frameSize = audioSettings.Value<unsigned int>(AudioModule::SettingNames::Framesize);
		const unsigned int channels = m_pAudioModule->Channels();

		float* outData = reinterpret_cast<float*>(stream);

		/* Deinterleave stream */
		iplAudioBufferDeinterleave(m_IPLContext, outData, &m_InBuffers[channel.m_Index]);
		IPLBinauralEffectParams effectParams{};
		effectParams.direction = IPLVector3{ dir.x, dir.y, dir.z };
		effectParams.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
		effectParams.spatialBlend = 1.0f;
		effectParams.hrtf = m_IPLHrtf;
		effectParams.peakDelays = nullptr;
		/* Apply effect */
		iplBinauralEffectApply(m_BinauralEffects[channel.m_Index], &effectParams, &m_InBuffers[channel.m_Index], &m_OutBuffers[channel.m_Index]);
		/* Interleave back into stream */
		iplAudioBufferInterleave(m_IPLContext, &m_OutBuffers[channel.m_Index], m_TemporaryBuffers[channel.m_Index].data());
		std::memcpy(stream, m_TemporaryBuffers[channel.m_Index].data(), len);
	}

	void SteamAudioModule::AllocateChannels(size_t mixingChannels)
	{
		ModuleSettings& audioModuleSettings = m_pAudioModule->Settings();
		const unsigned int frameSize = audioModuleSettings.Value<unsigned int>(AudioModule::SettingNames::Framesize);
		const unsigned int samplingRate = m_pAudioModule->SamplingRate();
		const unsigned int channels = m_pAudioModule->Channels();

		const size_t oldChannels = m_InBuffers.size();

		m_InBuffers.resize(mixingChannels);
		m_OutBuffers.resize(mixingChannels);
		m_BinauralEffects.resize(mixingChannels);
		m_TemporaryBuffers.resize(mixingChannels);

		IPLAudioSettings audioSettings{};
		audioSettings.samplingRate = samplingRate;
		audioSettings.frameSize = frameSize;

		IPLBinauralEffectSettings effectSettings{};
		effectSettings.hrtf = m_IPLHrtf;

		for (size_t i = oldChannels; i < m_InBuffers.size(); ++i)
		{
			m_TemporaryBuffers[i].resize(frameSize*channels);
			iplAudioBufferAllocate(m_IPLContext, channels, frameSize, &m_InBuffers[i]);
			iplAudioBufferAllocate(m_IPLContext, channels, frameSize, &m_OutBuffers[i]);
			iplBinauralEffectCreate(m_IPLContext, &audioSettings, &effectSettings, &m_BinauralEffects[i]);
		}
	}
}