#include "FSMModule.h"
#include "FSM.h"

#include <AssetManager.h>
#include <Engine.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(FSMModule);

	FSMModule::FSMModule()
	{
	}

	FSMModule::~FSMModule()
	{
	}

	const std::type_info& FSMModule::GetModuleType()
	{
		return typeid(FSMModule);
	}

	UUID FSMModule::CreateFSMState(FSMData* pData)
	{
		UUID id = UUID();
		FSMState& state = m_States.emplace_back(this, pData->GetUUID(), id);
		state.SetCurrentState(pData->StartNodeID());
		return id;
	}

	void FSMModule::CleanupStates()
	{
		m_States.clear();
	}

	void FSMModule::Initialize()
	{
		Utils::Reflect::Reflect::SetReflectInstance(&m_pEngine->Reflection());
		Reflect::RegisterType<FSMNode>();
		Reflect::RegisterType<FSMTransition>();
		m_pEngine->GetResourceTypes().RegisterResource<FSMData>("");
	}

	void FSMModule::PostInitialize()
	{
	}

	void FSMModule::Update()
	{
	}

	void FSMModule::Cleanup()
	{
	}

	void FSMModule::LoadSettings(ModuleSettings& settings)
	{
	}
}
