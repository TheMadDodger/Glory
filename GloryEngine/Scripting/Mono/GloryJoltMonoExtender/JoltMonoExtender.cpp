#include "JoltMonoExtender.h"
#include "PhysicsCSAPI.h"
#include "PhysicsComponentsCSAPI.h"
#include "MonoScriptedSystem.h"

#include <GloryMonoScipting.h>
#include <ScriptingExtender.h>
#include <PhysicsSystem.h>
#include <Engine.h>

namespace Glory
{
	JoltMonoExtender::JoltMonoExtender(const char* path) : m_pLibManager(new JoltLibManager()), m_Path(path)
	{
	}

	JoltMonoExtender::~JoltMonoExtender()
	{
		delete m_pLibManager;
		m_pLibManager = nullptr;
	}

	void JoltMonoExtender::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		PhysicsCSAPI::AddInternalCalls(internalCalls);
		PhysicsComponentsCSAPI::AddInternalCalls(internalCalls);
	}

	void JoltMonoExtender::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		pScriptingExtender->AddInternalLib(m_Path, "GloryEngine.Jolt.dll", m_pLibManager);
	}

	bool OnLoadExtra(const char* path, Module* pModule, Module* pRequiredModule)
	{
		GloryMonoScipting* pScripting = (GloryMonoScipting*)pRequiredModule;
		IScriptExtender* pScriptExtender = new JoltMonoExtender(path);
		pScripting->GetScriptingExtender()->RegisterExtender(pScriptExtender);
		return true;
	}

	void JoltLibManager::CollectTypes(Engine*, Assembly*)
	{
	}

	void JoltLibManager::Initialize(Engine* pEngine, Assembly*)
	{
		PhysicsCSAPI::SetEngine(pEngine);
		PhysicsComponentsCSAPI::SetEngine(pEngine);

		PhysicsSystem::Instance()->OnBodyActivated_Callback = MonoScriptedSystem::OnBodyActivated;
		PhysicsSystem::Instance()->OnBodyDeactivated_Callback = MonoScriptedSystem::OnBodyDeactivated;
		PhysicsSystem::Instance()->OnContactAdded_Callback = MonoScriptedSystem::OnContactAdded;
		PhysicsSystem::Instance()->OnContactPersisted_Callback = MonoScriptedSystem::OnContactPersisted;
		PhysicsSystem::Instance()->OnContactRemoved_Callback = MonoScriptedSystem::OnContactRemoved;
	}

	void JoltLibManager::Cleanup(Engine*)
	{
		PhysicsSystem::Instance()->OnBodyActivated_Callback = NULL;
		PhysicsSystem::Instance()->OnBodyDeactivated_Callback = NULL;
		PhysicsSystem::Instance()->OnContactAdded_Callback = NULL;
		PhysicsSystem::Instance()->OnContactPersisted_Callback = NULL;
		PhysicsSystem::Instance()->OnContactRemoved_Callback = NULL;
	}

	void JoltLibManager::Reset(Engine*)
	{
	}
}
