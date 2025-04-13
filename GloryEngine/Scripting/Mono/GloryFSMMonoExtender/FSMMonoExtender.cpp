#include "FSMMonoExtender.h"
#include "FSMCSAPI.h"

#include <MonoManager.h>
#include <AssemblyDomain.h>
#include <CoreLibManager.h>
#include <FSMModule.h>
#include <GloryMonoScipting.h>
#include <ScriptingExtender.h>
#include <Engine.h>
#include <Assembly.h>
#include <Debug.h>
#include <FSM.h>

namespace Glory
{
	FSMMonoExtender::FSMMonoExtender(const char* path) : m_pLibManager(new FSMLibManager()), m_Path(path)
	{
	}

	FSMMonoExtender::~FSMMonoExtender()
	{
		delete m_pLibManager;
		m_pLibManager = nullptr;
	}

	void FSMMonoExtender::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		FSMCSAPI::AddInternalCalls(internalCalls);
	}

	void FSMMonoExtender::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		pScriptingExtender->AddInternalLib(m_Path, "GloryEngine.FSM.dll", m_pLibManager);
	}

	bool OnLoadExtra(const char* path, Module* pModule, Module* pRequiredModule)
	{
		GloryMonoScipting* pScripting = (GloryMonoScipting*)pRequiredModule;
		IScriptExtender* pScriptExtender = new FSMMonoExtender(path);
		pScripting->GetScriptingExtender()->RegisterExtender(pScriptExtender);
		return true;
	}

	void FSMLibManager::Initialize(Engine* pEngine, Assembly* pAssembly)
	{
		m_pAssembly = pAssembly;

		FSMCSAPI::SetEngine(pEngine);
		AddMonoType("GloryEngine.FSM.FSMTemplate", SerializedType::ST_Asset, ResourceTypes::GetHash<FSMData>());

		MonoManager* pMonoManager = MonoManager::Instance();

		if (m_FSMGCHandle)
		{
			mono_gchandle_free(m_FSMGCHandle);
			pMonoManager->CollectGC();
			pMonoManager->WaitForPendingFinalizers();
			m_FSMGCHandle = 0;
		}

		AssemblyClass* pFSDMManagerClass = m_pAssembly->GetClass("GloryEngine.FSM", "FSMManager");

		m_pFSMManagerObject = mono_object_new(mono_domain_get(), pFSDMManagerClass->m_pClass);
		if (m_pFSMManagerObject == nullptr)
		{
			pEngine->GetDebug().LogError("FSMLibManager::Initialize > Failed to create FSM Manager MonoObject");
			return;
		}

		mono_runtime_object_init(m_pFSMManagerObject);
		m_pFSMReset = pFSDMManagerClass->GetMethod(".::Reset");
		m_FSMGCHandle = mono_gchandle_new(m_pFSMManagerObject, true);

		FSMModule* pFSMModule = pEngine->GetOptionalModule<FSMModule>();

		MonoMethod* pEntryMethod = pFSDMManagerClass->GetMethod(".::CallNodeEntry");
		MonoMethod* pExitMethod = pFSDMManagerClass->GetMethod(".::CallNodeExit");

		AssemblyDomain* pDomain = pAssembly->GetDomain();

		pFSMModule->EntryCallback =
		[pDomain, pEntryMethod](const FSMState& state, const FSMNode& node) {
			UUID nodeId = node.m_ID;
			void* args[] = {
				&state.ID(),
				&nodeId,
			};

			pDomain->InvokeMethod(pEntryMethod, nullptr, args);
		};

		pFSMModule->ExitCallback =
		[pDomain, pExitMethod](const FSMState& state, const FSMNode& node) {
			UUID nodeId = node.m_ID;
			void* args[] = {
				&state.ID(),
				&nodeId,
			};

			pDomain->InvokeMethod(pExitMethod, nullptr, args);
		};
	}

	void FSMLibManager::Cleanup(Engine*)
	{
	}

	void FSMLibManager::Reset(Engine*)
	{
		AssemblyClass* pFSDMManagerClass = m_pAssembly->GetClass("GloryEngine.FSM", "FSMManager");
		MonoMethod* pReset = pFSDMManagerClass->GetMethod(".::Reset");
		MonoObject* pExcept;
		MonoObject* pReturn = mono_runtime_invoke(pReset, m_pFSMManagerObject, nullptr, &pExcept);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);
	}
}
