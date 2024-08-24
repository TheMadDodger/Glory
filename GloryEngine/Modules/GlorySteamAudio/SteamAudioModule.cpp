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
#include <AudioSourceSystem.h>
#include <AudioComponents.h>
#include <AudioScene.h>
#include <SoundMaterialData.h>
#include <SoundComponents.h>

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

	IPLContext SteamAudioModule::GetContext()
	{
		return m_IPLContext;
	}

	void SteamAudioModule::AddAudioScene(AudioScene&& audioScene)
	{
		m_AudioScenes.push_back(std::move(audioScene));
		/** @todo: Keep track of which mesh came from which scene for easier removal when the scene unloads */
		/** @todo: Load audio scene when a GScene is loaded if available */
	}

	void SteamAudioModule::RemoveAllAudioScenes()
	{
		m_AudioScenes.clear();
	}

	void SteamAudioModule::RebuildAudioSimulationScene()
	{
		if (m_Scene)
			iplSceneRelease(&m_Scene);

		IPLSceneSettings sceneSettings{};
		sceneSettings.type = IPL_SCENETYPE_DEFAULT;
		iplSceneCreate(m_IPLContext, &sceneSettings, &m_Scene);

		iplSimulatorSetScene(m_Simulator, m_Scene);
		iplSimulatorCommit(m_Simulator);

		for (size_t i = 0; i < m_AudioScenes.size(); ++i)
		{
			AudioScene& scene = m_AudioScenes[i];

			for (size_t i = 0; i < scene.MeshCount(); ++i)
			{
				MeshData& mesh = scene.Mesh(i);
				const SoundMaterial& material = scene.Material(i);

				IPLMaterial materials[1];
				materials[0].absorption[0] = material.m_Absorption.x;
				materials[0].absorption[1] = material.m_Absorption.y;
				materials[0].absorption[2] = material.m_Absorption.z;
				materials[0].scattering = material.m_Scattering;
				materials[0].transmission[0] = material.m_Transmission.x;
				materials[0].transmission[1] = material.m_Transmission.y;
				materials[0].transmission[2] = material.m_Transmission.z;

				/* @todo: Generate material indices */
				IPLint32 materialIndices[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

				IPLStaticMeshSettings staticMeshSettings{};
				staticMeshSettings.numVertices = mesh.VertexCount();
				staticMeshSettings.numTriangles = mesh.IndexCount() / 3;
				staticMeshSettings.numMaterials = 1;
				staticMeshSettings.vertices = reinterpret_cast<IPLVector3*>(mesh.Vertices());
				staticMeshSettings.triangles = reinterpret_cast<IPLTriangle*>(mesh.Indices());
				staticMeshSettings.materialIndices = materialIndices;
				staticMeshSettings.materials = materials;

				IPLStaticMesh staticMesh = nullptr;
				iplStaticMeshCreate(m_Scene, &staticMeshSettings, &staticMesh);
				iplStaticMeshAdd(staticMesh, m_Scene);
			}
		}

		iplSceneCommit(m_Scene);
	}

	const SoundMaterial& SteamAudioModule::DefaultMaterial() const
	{
		return m_DefaultSoundMaterial;
	}

	SoundMaterial& SteamAudioModule::DefaultMaterial()
	{
		return m_DefaultSoundMaterial;
	}

	bool SteamAudioModule::HasFeature(uint32_t feature) const
	{
		switch (feature)
		{
		case Features::Spatialization:
		case Features::DirectSimulation:
		case Features::Occlusion:
		case Features::Absorption:
		case Features::Transmission:
			return true;
		case Features::ReflectionSimulation:
		case Features::PathingSimulation:
			return false;

		default:
			return false;
		}
	}

	void SteamAudioModule::Initialize()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());

		m_pEngine->Reflection().RegisterType<SoundMaterial>();
		m_pEngine->GetResourceTypes().RegisterResource<SoundMaterialData>(".gsmat");

		m_pEngine->GetSceneManager()->RegisterComponent<SoundOccluder>();

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

		IPLSimulationSettings simulationSettings{};
		/* Enable occlusion/transmission simulation */
		simulationSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;
		simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;
		iplSimulatorCreate(m_IPLContext, &simulationSettings, &m_Simulator);
		iplSimulatorSetScene(m_Simulator, m_Scene);
		iplSimulatorCommit(m_Simulator);

		m_pAudioModule->SourceSystem().OnSourceStart = [this](Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent) {
			AddSource(pRegistry, entity, pComponent);
		};

		m_pAudioModule->SourceSystem().OnSourceStop = [this](Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent) {
			RemoveSource(pRegistry, entity, pComponent);
		};

		m_pAudioModule->SourceSystem().OnSourceUpdate = [this](Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& pComponent) {
			UpdateSource(pRegistry, entity, pComponent);
		};

		m_pAudioModule->ListenerSystem().OnListenerUpdate = [this](Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioListener& pComponent) {
			if (!pComponent.m_Enable) return;

			Transform& transform = pRegistry->GetComponent<Transform>(entity);
			const glm::vec3 right = glm::vec3(transform.MatTransform[0][0], transform.MatTransform[1][0], transform.MatTransform[2][0]);
			const glm::vec3 up = glm::vec3(transform.MatTransform[0][1], transform.MatTransform[1][1], transform.MatTransform[2][1]);
			const glm::vec3 forward = glm::vec3(transform.MatTransform[0][2], transform.MatTransform[1][2], transform.MatTransform[2][2]);
			const glm::vec3 pos = glm::vec3(transform.MatTransform[3][0], transform.MatTransform[3][1], transform.MatTransform[3][2]);

			IPLCoordinateSpace3 listenerCoordinates;
			listenerCoordinates.origin = IPLVector3{ pos.x, pos.y, pos.z };
			listenerCoordinates.ahead = IPLVector3{ forward.x, forward.y, -forward.z };
			listenerCoordinates.right = IPLVector3{ right.x, right.y, right.z };
			listenerCoordinates.up = IPLVector3{ up.x, up.y, up.z };

			IPLSimulationSharedInputs sharedInputs{};
			sharedInputs.listener = listenerCoordinates;

			const IPLSimulationFlags simulationFlags = IPLSimulationFlags(
				pComponent.m_Simulation.m_Enable ? (
					(pComponent.m_Simulation.m_Direct ? IPL_SIMULATIONFLAGS_DIRECT : 0) |
					(pComponent.m_Simulation.m_Reflection ? IPL_SIMULATIONFLAGS_REFLECTIONS : 0) |
					(pComponent.m_Simulation.m_Pathing ? IPL_SIMULATIONFLAGS_PATHING : 0)) : 0);
			iplSimulatorSetSharedInputs(m_Simulator, simulationFlags, &sharedInputs);
		};
	}

	void SteamAudioModule::Update()
	{
		iplSimulatorRunDirect(m_Simulator);
	}

	void SteamAudioModule::Cleanup()
	{
		for (size_t i = 0; i < m_Sources.size(); ++i)
		{
			if (!m_Sources[i]) continue;
			iplSourceRemove(m_Sources[i], m_Simulator);
			iplSourceRelease(&m_Sources[i]);
		}
		m_Sources.clear();
		m_SourceEntities.clear();

		for (size_t i = 0; i < m_InBuffers.size(); ++i)
		{
			iplAudioBufferFree(m_IPLContext, &m_InBuffers[i]);
			iplAudioBufferFree(m_IPLContext, &m_OutBuffers[i]);
			iplAudioBufferFree(m_IPLContext, &m_AmbisonicsBuffers[i]);
			iplBinauralEffectRelease(&m_BinauralEffects[i]);
			iplDirectEffectRelease(&m_DirectEffects[i]);
			iplAmbisonicsEncodeEffectRelease(&m_AmbiSonicsEffects[i]);
			iplAmbisonicsDecodeEffectRelease(&m_AmbiSonicsDecodeEffects[i]);
		}

		iplSceneRelease(&m_Scene);
		iplSimulatorRelease(&m_Simulator);

		iplHRTFRelease(&m_IPLHrtf);
		iplContextRelease(&m_IPLContext);
	}

	float CalculateAttenuation(IPLfloat32 distance, void* userData)
	{
		return 1.0f - std::clamp(distance/500.0f, 0.0f, 1.0f);
	}

	void SteamAudioModule::SpatializeBinaural(AudioChannel& channel, const glm::vec3& dir, float spatialBlend, void* stream)
	{
		float* outData = reinterpret_cast<float*>(stream);

		/* Deinterleave stream */
		iplAudioBufferDeinterleave(m_IPLContext, outData, &m_InBuffers[channel.m_Index]);
		IPLBinauralEffectParams effectParams{};
		effectParams.direction = IPLVector3{ dir.x, dir.y, dir.z };
		effectParams.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
		effectParams.spatialBlend = spatialBlend;
		effectParams.hrtf = m_IPLHrtf;
		effectParams.peakDelays = nullptr;
		/* Apply effect */
		iplBinauralEffectApply(m_BinauralEffects[channel.m_Index], &effectParams, &m_InBuffers[channel.m_Index], &m_OutBuffers[channel.m_Index]);
	}

	void SteamAudioModule::SpatializeAmbisonics(AudioChannel& channel, const glm::vec3& listenPos, const glm::vec3& dir, int order, void* stream)
	{
		ModuleSettings& audioSettings = m_pAudioModule->Settings();
		const unsigned int frameSize = audioSettings.Value<unsigned int>(AudioModule::SettingNames::Framesize);
		const unsigned int channels = m_pAudioModule->Channels();

		if (m_AmbiSonicsOrders[channel.m_Index] != order)
		{
			iplAudioBufferFree(m_IPLContext, &m_AmbisonicsBuffers[channel.m_Index]);
			iplAudioBufferAllocate(m_IPLContext, pow(order + 1, 2), frameSize, &m_AmbisonicsBuffers[channel.m_Index]);
			m_AmbiSonicsOrders[channel.m_Index] = order;
		}

		float* outData = reinterpret_cast<float*>(stream);
		iplAudioBufferDeinterleave(m_IPLContext, outData, &m_InBuffers[channel.m_Index]);

		IPLAmbisonicsEncodeEffectParams ambiSonicsEncodeParams{};
		ambiSonicsEncodeParams.direction = IPLVector3{ dir.x, dir.y, dir.z };
		ambiSonicsEncodeParams.order = order;

		iplAmbisonicsEncodeEffectApply(m_AmbiSonicsEffects[channel.m_Index], &ambiSonicsEncodeParams, &m_InBuffers[channel.m_Index], &m_AmbisonicsBuffers[channel.m_Index]);

		IPLCoordinateSpace3 listenerCoordinates;
		listenerCoordinates.origin = IPLVector3{ listenPos.x, listenPos.y, listenPos.z };
		listenerCoordinates.ahead = IPLVector3{ 0.0f, 0.0f, -1.0f };
		listenerCoordinates.right = IPLVector3{ 1.0f, 0.0f, 0.0f };
		listenerCoordinates.up = IPLVector3{ 0.0f, 1.0f, 0.0f };

		IPLAmbisonicsDecodeEffectParams ambiSonicsDecodeParams{};
		ambiSonicsDecodeParams.order = order;
		ambiSonicsDecodeParams.hrtf = m_IPLHrtf;
		ambiSonicsDecodeParams.orientation = listenerCoordinates;
		ambiSonicsDecodeParams.binaural = channels > 2 ? IPL_FALSE : IPL_TRUE;

		iplAmbisonicsDecodeEffectApply(m_AmbiSonicsDecodeEffects[channel.m_Index], &ambiSonicsDecodeParams, &m_AmbisonicsBuffers[channel.m_Index], &m_OutBuffers[channel.m_Index]);
	}

	float SteamAudioModule::Attenuate(const glm::vec3& listenPos, const glm::vec3& sourcePos) const
	{
		IPLDistanceAttenuationModel distanceAttenuationModel{};
		distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_CALLBACK;
		distanceAttenuationModel.callback = CalculateAttenuation;
		return iplDistanceAttenuationCalculate(m_IPLContext, { sourcePos.x, sourcePos.y, sourcePos.z }, IPLVector3{ listenPos.x, listenPos.y, listenPos.z }, &distanceAttenuationModel);
	}

	void SteamAudioModule::ProcessEffects(AudioChannel& channel, void* stream, int len)
	{
		/* For now only audio source components support effects */
		if (channel.m_UserData.m_Type != AudioChannelUDataType::Entity) return;

		/* Get listsner information */
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
		GScene* pScene = m_pEngine->GetSceneManager()->GetOpenScene(channel.m_UserData.sceneID());
		if (!pScene) return;
		const UUID sourceID = channel.m_UserData.entityID();
		Entity entity = pScene->GetEntityByUUID(sourceID);
		if (!entity.IsValid()) return;
		const glm::mat4 source = entity.GetComponent<Transform>().MatTransform;
		glm::quat sourceRot;
		glm::decompose(source, scale, sourceRot, sourcePos, skew, pers);
		dir = sourcePos - listenPos;
		dir = dir * listenRot;

		const AudioSource& audioSource = entity.GetComponent<AudioSource>();
		/* No need to bother doing any copying if spatialization and simulation are off */
		if (!audioSource.m_Spatialization.m_Enable && !audioSource.m_Simulation.m_Enable) return;

		if (audioSource.m_Spatialization.m_Enable)
		{
			switch (audioSource.m_Spatialization.m_Mode)
			{
			case Glory::SpatializationMode::Binaural:
				SpatializeBinaural(channel, dir, audioSource.m_Spatialization.m_SpatialBlend, stream);
				break;
			case Glory::SpatializationMode::Ambisonics:
				SpatializeAmbisonics(channel, listenPos, dir, int(audioSource.m_Spatialization.m_AmbisonicsOrder) + 1, stream);
				break;
			default:
				break;
			}
		}
		else
		{
			/* We still need to deinterleave the stream to apply the simulation */
			float* outData = reinterpret_cast<float*>(stream);
			iplAudioBufferDeinterleave(m_IPLContext, outData, &m_OutBuffers[channel.m_Index]);
		}

		if (!audioSource.m_Simulation.m_Enable)
		{
			/* If spatialization had attenuation on we still need to apply it here */
			if (audioSource.m_Spatialization.m_Attenuation.m_Enable)
			{
				IPLDirectEffectParams directAttenuateEffect;
				directAttenuateEffect.flags = IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION;
				directAttenuateEffect.distanceAttenuation = Attenuate(listenPos, sourcePos);
				iplDirectEffectApply(m_DirectEffects[channel.m_Index], &directAttenuateEffect,
					&m_OutBuffers[channel.m_Index], &m_InBuffers[channel.m_Index]);

				/* Interleave back into stream */
				iplAudioBufferInterleave(m_IPLContext, &m_InBuffers[channel.m_Index], m_TemporaryBuffers[channel.m_Index].data());
				std::memcpy(stream, m_TemporaryBuffers[channel.m_Index].data(), len);
				return;
			}

			/* Interleave back into stream */
			iplAudioBufferInterleave(m_IPLContext, &m_OutBuffers[channel.m_Index], m_TemporaryBuffers[channel.m_Index].data());
			std::memcpy(stream, m_TemporaryBuffers[channel.m_Index].data(), len);
			return;
		}

		auto iter = std::find(m_SourceEntities.begin(), m_SourceEntities.end(), sourceID);
		/* The engine could be shutting down so just early return to prevent crashing */
		if (iter == m_SourceEntities.end()) return;
		const size_t sourceIndex = iter - m_SourceEntities.begin();

		const DirectSimulationSettings& directSettings = audioSource.m_Simulation.m_Direct;
		const IPLSimulationFlags simulationFlags = IPLSimulationFlags(
			audioSource.m_Simulation.m_Enable ? (
				(directSettings.m_Enable ? IPL_SIMULATIONFLAGS_DIRECT : 0) |
				(audioSource.m_Simulation.m_Reflections.m_Enable ? IPL_SIMULATIONFLAGS_REFLECTIONS : 0) |
				(audioSource.m_Simulation.m_Pathing.m_Enable ? IPL_SIMULATIONFLAGS_PATHING : 0)) : 0);

		const AttenuationSettings& attenuationSettings = directSettings.m_DistanceAttenuation;
		const AirAbsorptionSettings& airAbsorptionSettings = directSettings.m_AirAbsorption;
		const DirectivitySettings& directivitySettings = directSettings.m_Directivity;
		const OcclusionSettings& occlusionSettings = directSettings.m_Occlusion;
		const TransmissionSettings& transmissionSettings = directSettings.m_Transmission;
		const IPLDirectEffectFlags directFlags = IPLDirectEffectFlags(
			directSettings.m_Enable ? (
				(attenuationSettings.m_Enable ? IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION : 0) |
				(airAbsorptionSettings.m_Enable ? IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION : 0) |
				(directivitySettings.m_Enable ? IPL_DIRECTEFFECTFLAGS_APPLYDIRECTIVITY : 0) |
				(occlusionSettings.m_Enable ? IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION : 0) |
				(transmissionSettings.m_Enable ? IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION : 0)) : 0);

		IPLSimulationOutputs outputs{};
		iplSourceGetOutputs(m_Sources[sourceIndex], simulationFlags, &outputs);

		IPLDirectEffectParams directEffectsParams = outputs.direct;
		directEffectsParams.flags = directFlags;

		if (audioSource.m_Spatialization.m_Enable && audioSource.m_Spatialization.m_Attenuation.m_Enable)
		{
			directEffectsParams.flags = IPLDirectEffectFlags(int(directEffectsParams.distanceAttenuation) |
				IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION);
			directEffectsParams.distanceAttenuation = Attenuate(listenPos, sourcePos);
		}

		directEffectsParams.transmissionType = transmissionSettings.m_FrequencyDependant ?
			IPL_TRANSMISSIONTYPE_FREQDEPENDENT : IPL_TRANSMISSIONTYPE_FREQINDEPENDENT;

		iplDirectEffectApply(m_DirectEffects[channel.m_Index], &directEffectsParams, &m_OutBuffers[channel.m_Index], &m_InBuffers[channel.m_Index]);

		/* Interleave back into stream */
		iplAudioBufferInterleave(m_IPLContext, &m_InBuffers[channel.m_Index], m_TemporaryBuffers[channel.m_Index].data());
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
		m_AmbisonicsBuffers.resize(mixingChannels);
		m_AmbiSonicsOrders.resize(mixingChannels, 2);
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
		ambiSonicsEffectSettings.maxOrder = 3; // 2nd order Ambisonics (9 channels)

		IPLAmbisonicsDecodeEffectSettings ambiSonicsDecodeSettings{};
		ambiSonicsDecodeSettings.maxOrder = 3;
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
		directEffectSettings.numChannels = channels;

		for (size_t i = oldChannels; i < mixingChannels; ++i)
		{
			m_TemporaryBuffers[i].resize(frameSize*channels);
			iplAudioBufferAllocate(m_IPLContext, channels, frameSize, &m_InBuffers[i]);
			iplAudioBufferAllocate(m_IPLContext, channels, frameSize, &m_OutBuffers[i]);
			const int ambisonicsChannels = pow(m_AmbiSonicsOrders[i] + 1, 2);
			iplAudioBufferAllocate(m_IPLContext, ambisonicsChannels, frameSize, &m_AmbisonicsBuffers[i]);
			iplBinauralEffectCreate(m_IPLContext, &audioSettings, &effectSettings, &m_BinauralEffects[i]);
			iplDirectEffectCreate(m_IPLContext, &audioSettings, &directEffectSettings, &m_DirectEffects[i]);
			iplAmbisonicsEncodeEffectCreate(m_IPLContext, &audioSettings, &ambiSonicsEffectSettings, &m_AmbiSonicsEffects[i]);
			iplAmbisonicsDecodeEffectCreate(m_IPLContext, &audioSettings, &ambiSonicsDecodeSettings, &m_AmbiSonicsDecodeEffects[i]);
		}
	}

	void SteamAudioModule::AddSource(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& audioSource)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		const UUID id = pScene->GetEntityUUID(entity);

		auto iter = std::find(m_SourceEntities.begin(), m_SourceEntities.end(), 0);
		size_t sourceIndex = 0;
		if (iter == m_SourceEntities.end())
		{
			sourceIndex = m_Sources.size();
			m_Sources.push_back(nullptr);
			m_SourceEntities.push_back(id);
		}
		else
		{
			sourceIndex = iter - m_SourceEntities.begin();
			m_SourceEntities[sourceIndex] = id;
		}

		const IPLSimulationFlags simulationFlags = IPLSimulationFlags(
			audioSource.m_Simulation.m_Enable ? (
				(audioSource.m_Simulation.m_Direct.m_Enable ? IPL_SIMULATIONFLAGS_DIRECT : 0) |
				(audioSource.m_Simulation.m_Reflections.m_Enable ? IPL_SIMULATIONFLAGS_REFLECTIONS : 0) |
				(audioSource.m_Simulation.m_Pathing.m_Enable ? IPL_SIMULATIONFLAGS_PATHING : 0)) : 0);

		IPLSourceSettings sourceSettings{};
		sourceSettings.flags = simulationFlags; // this enables occlusion/transmission simulator for this source

		iplSourceCreate(m_Simulator, &sourceSettings, &m_Sources[sourceIndex]);

		iplSourceAdd(m_Sources[sourceIndex], m_Simulator);
		iplSimulatorCommit(m_Simulator);

		UpdateSource(pRegistry, entity, audioSource);
	}

	void SteamAudioModule::UpdateSource(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& audioSource)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		const UUID id = pScene->GetEntityUUID(entity);

		auto iter = std::find(m_SourceEntities.begin(), m_SourceEntities.end(), id);
		if (iter == m_SourceEntities.end()) return;
		const size_t index = iter - m_SourceEntities.begin();
		IPLSource source = m_Sources[index];

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		const glm::vec3 right = glm::vec3(transform.MatTransform[0][0], transform.MatTransform[1][0], transform.MatTransform[2][0]);
		const glm::vec3 up = glm::vec3(transform.MatTransform[0][1], transform.MatTransform[1][1], transform.MatTransform[2][1]);
		const glm::vec3 forward = glm::vec3(transform.MatTransform[0][2], transform.MatTransform[1][2], transform.MatTransform[2][2]);
		const glm::vec3 pos = glm::vec3(transform.MatTransform[3][0], transform.MatTransform[3][1], transform.MatTransform[3][2]);

		IPLCoordinateSpace3 sourceCoordinates;
		sourceCoordinates.origin = IPLVector3{ pos.x, pos.y, pos.z };
		sourceCoordinates.ahead = IPLVector3{ forward.x, forward.y, -forward.z };
		sourceCoordinates.right = IPLVector3{ right.x, right.y, right.z };
		sourceCoordinates.up = IPLVector3{ up.x, up.y, up.z };

		const DirectSimulationSettings& directSettings = audioSource.m_Simulation.m_Direct;
		const IPLSimulationFlags simulationFlags = IPLSimulationFlags(
			audioSource.m_Simulation.m_Enable ? (
				(directSettings.m_Enable ? IPL_SIMULATIONFLAGS_DIRECT : 0) |
				(audioSource.m_Simulation.m_Reflections.m_Enable ? IPL_SIMULATIONFLAGS_REFLECTIONS : 0) |
				(audioSource.m_Simulation.m_Pathing.m_Enable ? IPL_SIMULATIONFLAGS_PATHING : 0)) : 0);

		const AttenuationSettings& attenuationSettings = directSettings.m_DistanceAttenuation;
		const AirAbsorptionSettings& airAbsorptionSettings = directSettings.m_AirAbsorption;
		const DirectivitySettings& directivitySettings = directSettings.m_Directivity;
		const OcclusionSettings& occlusionSettings = directSettings.m_Occlusion;
		const TransmissionSettings& transmissionSettings = directSettings.m_Transmission;
		const IPLDirectSimulationFlags directFlags = IPLDirectSimulationFlags(
			directSettings.m_Enable ? (
				(attenuationSettings.m_Enable ? IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION : 0) |
				(airAbsorptionSettings.m_Enable ? IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION : 0) |
				(directivitySettings.m_Enable ? IPL_DIRECTSIMULATIONFLAGS_DIRECTIVITY : 0) |
				(occlusionSettings.m_Enable ? IPL_DIRECTSIMULATIONFLAGS_OCCLUSION : 0) |
				(transmissionSettings.m_Enable ? IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION : 0)) : 0);

		IPLSimulationInputs inputs{};
		inputs.flags = simulationFlags;
		inputs.directFlags = directFlags;
		inputs.source = sourceCoordinates;

		/* Direct simulation settings */
		if (attenuationSettings.m_Enable)
		{
			inputs.distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_CALLBACK;
			inputs.distanceAttenuationModel.minDistance = attenuationSettings.m_MinDistance;
			inputs.distanceAttenuationModel.callback = CalculateAttenuation;
		}

		if (airAbsorptionSettings.m_Enable)
		{
			inputs.airAbsorptionModel.type = IPLAirAbsorptionModelType(airAbsorptionSettings.m_Type);
			inputs.airAbsorptionModel.coefficients[0] = airAbsorptionSettings.m_LowCoefficient;
			inputs.airAbsorptionModel.coefficients[1] = airAbsorptionSettings.m_MidCoefficient;
			inputs.airAbsorptionModel.coefficients[2] = airAbsorptionSettings.m_HighCoefficient;
		}

		if (directivitySettings.m_Enable)
		{
			inputs.directivity.dipoleWeight = directivitySettings.m_DipoleWeight;
			inputs.directivity.dipolePower = directivitySettings.m_DipolePower;
		}

		if (occlusionSettings.m_Enable)
		{
			inputs.occlusionType = IPLOcclusionType(occlusionSettings.m_Type);
			inputs.occlusionRadius = occlusionSettings.m_VolumetricRadius;
			inputs.numOcclusionSamples = occlusionSettings.m_VolumetricSamples;
		}

		if (transmissionSettings.m_Enable)
		{
			inputs.numTransmissionRays = transmissionSettings.m_TransmissionRays;
		}

		/* Reflection simulation settings */
		/* @todo */

		/* Pathing simulation settings */
		/* @todo */

		iplSourceSetInputs(source, simulationFlags, &inputs);
	}

	void SteamAudioModule::RemoveSource(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, AudioSource& audioSource)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		const UUID id = pScene->GetEntityUUID(entity);

		auto iter = std::find(m_SourceEntities.begin(), m_SourceEntities.end(), id);
		if (iter == m_SourceEntities.end()) return;
		const size_t index = iter - m_SourceEntities.begin();

		iplSourceRemove(m_Sources[index], m_Simulator);
		iplSimulatorCommit(m_Simulator);

		iplSourceRelease(&m_Sources[index]);
		m_Sources[index] = nullptr;
		m_SourceEntities[index] = 0;
	}
}