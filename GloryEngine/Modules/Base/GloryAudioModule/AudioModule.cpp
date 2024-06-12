#include "AudioModule.h"
#include "AudioComponents.h"
#include "AudioEmitterSystem.h"

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

		m_pEngine->GetSceneManager()->RegisterComponent<AudioEmitter>();

		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		pComponentTypes->RegisterInvokaction<AudioEmitter>(Glory::Utils::ECS::InvocationType::Update, AudioEmitterSystem::OnUpdate);
		pComponentTypes->RegisterInvokaction<AudioEmitter>(Glory::Utils::ECS::InvocationType::Start, AudioEmitterSystem::OnStart);
		pComponentTypes->RegisterInvokaction<AudioEmitter>(Glory::Utils::ECS::InvocationType::Stop, AudioEmitterSystem::OnStop);
	}
}