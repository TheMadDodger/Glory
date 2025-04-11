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
		FSMState& state = m_States.emplace_back(this, pData, id);
		state.SetCurrentState(pData->StartNodeID());
		return id;
	}

	FSMState* Glory::FSMModule::GetFSMState(UUID id)
	{
		for (size_t i = 0; i < m_States.size(); ++i)
		{
			if (m_States[i].ID() != id) continue;
			return &m_States[i];
		}
		return nullptr;
	}

	void FSMModule::DestroyFSMState(UUID id)
	{
		for (size_t i = 0; i < m_States.size(); ++i)
		{
			if (m_States[i].ID() != id) continue;
			m_States.erase(m_States.begin() + i);
			return;
		}
	}

	void FSMModule::CleanupStates()
	{
		m_States.clear();
	}

	void FSMModule::Initialize()
	{
		Utils::Reflect::Reflect::SetReflectInstance(&m_pEngine->Reflection());
		Reflect::RegisterEnum<FSMPropertyType>();
		Reflect::RegisterEnum<FSMTransitionOP>();
		Reflect::RegisterType<FSMNode>();
		Reflect::RegisterType<FSMTransition>();
		Reflect::RegisterType<FSMProperty>();
		m_pEngine->GetResourceTypes().RegisterResource<FSMData>("");
	}

	void FSMModule::PostInitialize()
	{
	}

	void FSMModule::Update()
	{
		for (size_t i = 0; i < m_States.size(); ++i)
		{
			m_States[i].Update();
		}
	}

	void FSMModule::Cleanup()
	{
	}

	void FSMModule::LoadSettings(ModuleSettings& settings)
	{
	}
}
