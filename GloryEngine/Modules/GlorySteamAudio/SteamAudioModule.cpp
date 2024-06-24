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

		IPLSceneSettings sceneSettings{};
		sceneSettings.type = IPL_SCENETYPE_DEFAULT;
		iplSceneCreate(m_IPLContext, &sceneSettings, &m_Scene);

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

		iplSceneRelease(&m_Scene);

		iplHRTFRelease(&m_IPLHrtf);
		iplContextRelease(&m_IPLContext);
	}

	float CalculateAttenuation(IPLfloat32 distance, void* userData)
	{
		if (distance <= 10.0f) return 1.0f;
		return 1.0f - std::clamp(distance/500.0f, 0.0f, 1.0f);
	}

	void SteamAudioModule::ProcessEffects(AudioChannel& channel, void* stream, int len)
	{
		glm::vec3 dir{};
		const glm::mat4 listener = m_pAudioModule->ListenerTransform();
		glm::vec3 scale, listenPos, skew;
		glm::vec4 pers;
		glm::quat listenRot;
		glm::decompose(listener, scale, listenRot, listenPos, skew, pers);

		const glm::vec3 right = glm::vec3(listener[0][0], listener[1][0], listener[2][0]);
		const glm::vec3 up = glm::vec3(listener[0][1], listener[1][1], listener[2][1]);
		const glm::vec3 forward = glm::vec3(listener[0][2], listener[1][2], listener[2][2]);

		glm::vec3 sourcePos{};
		switch (channel.m_UserData.m_Type)
		{
		case AudioChannelUDataType::Entity:
		{
			GScene* pScene = m_pEngine->GetSceneManager()->GetOpenScene(channel.m_UserData.sceneID());
			if (!pScene) return;
			Entity entity = pScene->GetEntityByUUID(channel.m_UserData.entityID());
			if (!entity.IsValid()) return;
			const glm::mat4 source = entity.GetComponent<Transform>().MatTransform;
			glm::quat sourceRot;
			glm::decompose(source, scale, sourceRot, sourcePos, skew, pers);
			dir = sourcePos - listenPos;
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
		iplAudioBufferDeinterleave(m_IPLContext, outData, &m_InBuffers[channel.m_Index]);

		IPLAmbisonicsEncodeEffectParams ambiSonicsEncodeParams{};
		ambiSonicsEncodeParams.direction = IPLVector3{ dir.x, dir.y, dir.z };
		ambiSonicsEncodeParams.order = 2;

		iplAmbisonicsEncodeEffectApply(m_AmbiSonicsEffects[channel.m_Index], &ambiSonicsEncodeParams, &m_InBuffers[channel.m_Index], &m_AmbisonicsBuffers[channel.m_Index]);

		IPLCoordinateSpace3 listenerCoordinates; // the listener's coordinate system
		listenerCoordinates.origin = IPLVector3{ listenPos.x, listenPos.y, listenPos.z };
		listenerCoordinates.ahead = IPLVector3{ 0.0f, 0.0f, -1.0f };
		listenerCoordinates.right = IPLVector3{ 1.0f, 0.0f, 0.0f };
		listenerCoordinates.up = IPLVector3{ 0.0f, 1.0f, 0.0f };

		IPLAmbisonicsDecodeEffectParams ambiSonicsDecodeParams{};
		ambiSonicsDecodeParams.order = 2;
		ambiSonicsDecodeParams.hrtf = m_IPLHrtf;
		ambiSonicsDecodeParams.orientation = listenerCoordinates;
		ambiSonicsDecodeParams.binaural = channels > 2 ? IPL_FALSE : IPL_TRUE;

		iplAmbisonicsDecodeEffectApply(m_AmbiSonicsDecodeEffects[channel.m_Index], &ambiSonicsDecodeParams, &m_AmbisonicsBuffers[channel.m_Index], &m_OutBuffers[channel.m_Index]);

		IPLDistanceAttenuationModel distanceAttenuationModel{};
		distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_CALLBACK;
		distanceAttenuationModel.minDistance = -10.0f;
		distanceAttenuationModel.callback = CalculateAttenuation;
		const float distanceAttenuation = iplDistanceAttenuationCalculate(m_IPLContext, { sourcePos.x, sourcePos.y, sourcePos.z }, listenerCoordinates.origin, &distanceAttenuationModel);

		IPLDirectEffectParams directEffectsParams{};
		directEffectsParams.flags = IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION;
		directEffectsParams.distanceAttenuation = distanceAttenuation;

		iplDirectEffectApply(m_DirectEffects[channel.m_Index], &directEffectsParams, &m_OutBuffers[channel.m_Index], &m_InBuffers[channel.m_Index]);

		iplAudioBufferInterleave(m_IPLContext, &m_InBuffers[channel.m_Index], m_TemporaryBuffers[channel.m_Index].data());
		std::memcpy(stream, m_TemporaryBuffers[channel.m_Index].data(), len);

		/* Deinterleave stream */
		//iplAudioBufferDeinterleave(m_IPLContext, outData, &m_InBuffers[channel.m_Index]);
		//IPLBinauralEffectParams effectParams{};
		//effectParams.direction = IPLVector3{ dir.x, dir.y, dir.z };
		//effectParams.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
		//effectParams.spatialBlend = 1.0f;
		//effectParams.hrtf = m_IPLHrtf;
		//effectParams.peakDelays = nullptr;
		///* Apply effect */
		//iplBinauralEffectApply(m_BinauralEffects[channel.m_Index], &effectParams, &m_InBuffers[channel.m_Index], &m_OutBuffers[channel.m_Index]);
		///* Interleave back into stream */
		//iplAudioBufferInterleave(m_IPLContext, &m_OutBuffers[channel.m_Index], m_TemporaryBuffers[channel.m_Index].data());
		//std::memcpy(stream, m_TemporaryBuffers[channel.m_Index].data(), len);
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
		m_AmbisonicsBuffers.resize(mixingChannels);
		m_BinauralEffects.resize(mixingChannels);
		m_DirectEffects.resize(mixingChannels);
		m_AmbiSonicsEffects.resize(mixingChannels);
		m_AmbiSonicsDecodeEffects.resize(mixingChannels);
		m_TemporaryBuffers.resize(mixingChannels);

		IPLAudioSettings audioSettings{};
		audioSettings.samplingRate = samplingRate;
		audioSettings.frameSize = frameSize;

		IPLBinauralEffectSettings effectSettings{};
		effectSettings.hrtf = m_IPLHrtf;

		IPLAmbisonicsEncodeEffectSettings ambiSonicsEffectSettings{};
		ambiSonicsEffectSettings.maxOrder = 2; // 2nd order Ambisonics (9 channels)

		IPLAmbisonicsDecodeEffectSettings ambiSonicsDecodeSettings{};
		ambiSonicsDecodeSettings.maxOrder = 2;
		ambiSonicsDecodeSettings.hrtf = m_IPLHrtf;

		switch (channels)
		{
		case 1:
			/* Actually shouldn't be allowed but I should probably disable
			 * the module completely if the audio module is running in mono */
			ambiSonicsDecodeSettings.speakerLayout.type = IPL_SPEAKERLAYOUTTYPE_MONO;
			break;
		case 2:
			ambiSonicsDecodeSettings.speakerLayout.type = IPL_SPEAKERLAYOUTTYPE_STEREO;
			break;
		case 4:
			ambiSonicsDecodeSettings.speakerLayout.type = IPL_SPEAKERLAYOUTTYPE_QUADRAPHONIC;
			break;
		case 6:
			ambiSonicsDecodeSettings.speakerLayout.type = IPL_SPEAKERLAYOUTTYPE_SURROUND_5_1;
			break;
		case 8:
			ambiSonicsDecodeSettings.speakerLayout.type = IPL_SPEAKERLAYOUTTYPE_SURROUND_7_1;
			break;

		default:
			ambiSonicsDecodeSettings.speakerLayout.type = IPL_SPEAKERLAYOUTTYPE_CUSTOM;
			break;
		}

		IPLDirectEffectSettings directEffectSettings{};
		directEffectSettings.numChannels = channels; // input and output buffers will have 1 channel

		for (size_t i = oldChannels; i < m_InBuffers.size(); ++i)
		{
			m_TemporaryBuffers[i].resize(frameSize*channels);
			iplAudioBufferAllocate(m_IPLContext, channels, frameSize, &m_InBuffers[i]);
			iplAudioBufferAllocate(m_IPLContext, channels, frameSize, &m_OutBuffers[i]);
			iplAudioBufferAllocate(m_IPLContext, 9, frameSize, &m_AmbisonicsBuffers[i]);
			iplBinauralEffectCreate(m_IPLContext, &audioSettings, &effectSettings, &m_BinauralEffects[i]);
			iplDirectEffectCreate(m_IPLContext, &audioSettings, &directEffectSettings, &m_DirectEffects[i]);
			iplAmbisonicsEncodeEffectCreate(m_IPLContext, &audioSettings, &ambiSonicsEffectSettings, &m_AmbiSonicsEffects[i]);
			iplAmbisonicsDecodeEffectCreate(m_IPLContext, &audioSettings, &ambiSonicsDecodeSettings, &m_AmbiSonicsDecodeEffects[i]);
		}
	}
}