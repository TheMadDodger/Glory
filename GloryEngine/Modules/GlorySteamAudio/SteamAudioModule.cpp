#include "SteamAudioModule.h"
#include "SoundMaterialData.h"

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

				/* @todo: Materials should come from a separate component, maybe even a resource? */
				IPLMaterial materials[1] = {
					{ {0.5f, 0.5f, 0.5f}, 0.5f, {0.1f, 0.1f, 0.1f} }
				};

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
		simulationSettings.flags = IPL_SIMULATIONFLAGS_DIRECT; // this enables occlusion/transmission simulation
		simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;
		// see below for examples of how to initialize the remaining fields of this structure
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
			if (!pComponent.m_Enabled) return;

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

			iplSimulatorSetSharedInputs(m_Simulator, IPL_SIMULATIONFLAGS_DIRECT, &sharedInputs);
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

		UUID sourceID = 0;
		glm::vec3 sourcePos{};
		switch (channel.m_UserData.m_Type)
		{
		case AudioChannelUDataType::Entity:
		{
			GScene* pScene = m_pEngine->GetSceneManager()->GetOpenScene(channel.m_UserData.sceneID());
			if (!pScene) return;
			sourceID = channel.m_UserData.entityID();
			Entity entity = pScene->GetEntityByUUID(sourceID);
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

		auto iter = std::find(m_SourceEntities.begin(), m_SourceEntities.end(), sourceID);
		/* The engine could be shutting down so just early return to prevent crashing */
		if (iter == m_SourceEntities.end()) return;
		const size_t sourceIndex = iter - m_SourceEntities.begin();

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
		distanceAttenuationModel.callback = CalculateAttenuation;
		const float distanceAttenuation = iplDistanceAttenuationCalculate(m_IPLContext, { sourcePos.x, sourcePos.y, sourcePos.z }, listenerCoordinates.origin, &distanceAttenuationModel);

		IPLSimulationOutputs outputs{};
		iplSourceGetOutputs(m_Sources[sourceIndex], IPL_SIMULATIONFLAGS_DIRECT, &outputs);

		IPLDirectEffectParams directEffectsParams = outputs.direct; // this can be passed to a direct 
		directEffectsParams.flags = IPLDirectEffectFlags(IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION | IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION | IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
		directEffectsParams.distanceAttenuation = distanceAttenuation;
		directEffectsParams.transmissionType = IPL_TRANSMISSIONTYPE_FREQDEPENDENT;

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

		IPLSourceSettings sourceSettings{};
		sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT; // this enables occlusion/transmission simulator for this source

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

		IPLSimulationInputs inputs{};
		inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
		inputs.directFlags = IPLDirectSimulationFlags(IPL_DIRECTSIMULATIONFLAGS_OCCLUSION | IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION | IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION);
		inputs.source = sourceCoordinates;
		inputs.occlusionType = IPL_OCCLUSIONTYPE_VOLUMETRIC;
		inputs.numTransmissionRays = 2;

		iplSourceSetInputs(source, IPL_SIMULATIONFLAGS_DIRECT, &inputs);
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