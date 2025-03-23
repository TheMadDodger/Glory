#include "AudioModule.h"
#include "AudioComponents.h"
#include "AudioSourceSystem.h"

#include <Engine.h>
#include <SceneManager.h>

#include <EntityRegistry.h>
#include <EntityID.h>
#include <ComponentTypes.h>

namespace Glory
{
	AudioModule::AudioModule(): m_AudioSourceSystem(new AudioSourceSystem), m_AudioListenerSystem(new AudioListenerSystem) {}
	AudioModule::~AudioModule() {}

	const std::type_info& AudioModule::GetBaseModuleType()
	{
		return typeid(AudioModule);
	}

	void AudioModule::Initialize()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());

		Reflect::RegisterEnum<SpatializationMode>();
		Reflect::RegisterEnum<AmbisonicsOrder>();
		Reflect::RegisterEnum<OcclusionType>();
		Reflect::RegisterEnum<AirAbsorptionType>();
		Reflect::RegisterType<SpatializationSettings>();
		Reflect::RegisterType<AttenuationSettings>();
		Reflect::RegisterType<AirAbsorptionSettings>();
		Reflect::RegisterType<DirectivitySettings>();
		Reflect::RegisterType<OcclusionSettings>();
		Reflect::RegisterType<TransmissionSettings>();
		Reflect::RegisterType<DirectSimulationSettings>();
		Reflect::RegisterType<ReflectionSimulationSettings>();
		Reflect::RegisterType<PathingSimulationSettings>();
		Reflect::RegisterType<AudioSourceSimulationSettings>();

		Reflect::RegisterType<AudioSimulationSettings>();

		m_pEngine->GetSceneManager()->RegisterComponent<AudioSource>();
		m_pEngine->GetSceneManager()->RegisterComponent<AudioListener>();

		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::OnValidate, AudioSourceSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::OnRemove, AudioSourceSystem::OnRemove);
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::Update, AudioSourceSystem::OnUpdate);
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::Start, AudioSourceSystem::OnStart);
		pComponentTypes->RegisterReferencesCallback<AudioSource>(AudioSourceSystem::GetReferences);
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::Stop, AudioSourceSystem::OnStop);
		pComponentTypes->RegisterInvokaction<AudioListener>(Glory::Utils::ECS::InvocationType::Update, AudioListenerSystem::OnUpdate);
		pComponentTypes->RegisterInvokaction<AudioListener>(Glory::Utils::ECS::InvocationType::Start, AudioListenerSystem::OnStart);
		pComponentTypes->RegisterInvokaction<AudioListener>(Glory::Utils::ECS::InvocationType::Stop, AudioListenerSystem::OnStop);
	}

	void AudioModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterValue<unsigned int>(SettingNames::MixingChannels, 64);
		settings.RegisterValue<unsigned int>(SettingNames::SamplingRate, 48000);
		settings.RegisterValue<unsigned int>(SettingNames::Framesize, 1024);
	}

	AudioSourceSystem& AudioModule::SourceSystem()
	{
		return *m_AudioSourceSystem;
	}

	AudioListenerSystem& AudioModule::ListenerSystem()
	{
		return *m_AudioListenerSystem;
	}
}