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
	AudioModule::AudioModule() {}
	AudioModule::~AudioModule() {}

	const std::type_info& AudioModule::GetBaseModuleType()
	{
		return typeid(AudioModule);
	}

	void AudioModule::Initialize()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());

		m_pEngine->GetSceneManager()->RegisterComponent<AudioSource>();

		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::OnValidate, AudioSourceSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::OnRemove, AudioSourceSystem::OnRemove);
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::Update, AudioSourceSystem::OnUpdate);
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::Start, AudioSourceSystem::OnStart);
		pComponentTypes->RegisterInvokaction<AudioSource>(Glory::Utils::ECS::InvocationType::Stop, AudioSourceSystem::OnStop);
	}
}