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

		//MonoMethod* pHoverMethod = pInteractionHandlerClass->GetMethod(".::OnElementHover");
		//MonoMethod* pUnhoverMethod = pInteractionHandlerClass->GetMethod(".::OnElementUnHover");
		//MonoMethod* pDownMethod = pInteractionHandlerClass->GetMethod(".::OnElementDown");
		//MonoMethod* pUpMethod = pInteractionHandlerClass->GetMethod(".::OnElementUp");

		//AssemblyDomain* pDomain = pAssembly->GetDomain();

		/*UIInteractionSystem::Instance()->OnElementHover_Callback =
		[pDomain, pHoverMethod](Engine* pEngine, UUID sceneID, UUID objectID, UUID elementID, UUID componentID) {
			MonoObject* pEngineObject = MonoManager::Instance()->GetCoreLibManager()->GetEngine();
			void* args[] = {
				pEngineObject,
				&sceneID,
				&objectID,
				&elementID,
				&componentID
			};

			pDomain->InvokeMethod(pHoverMethod, nullptr, args);
		};

		UIInteractionSystem::Instance()->OnElementUnHover_Callback =
		[pDomain, pUnhoverMethod](Engine* pEngine, UUID sceneID, UUID objectID, UUID elementID, UUID componentID) {
			MonoObject* pEngineObject = MonoManager::Instance()->GetCoreLibManager()->GetEngine();
			void* args[] = {
				pEngineObject,
				&sceneID,
				&objectID,
				&elementID,
				&componentID
			};

			pDomain->InvokeMethod(pUnhoverMethod, nullptr, args);
		};

		UIInteractionSystem::Instance()->OnElementDown_Callback =
		[pDomain, pDownMethod](Engine* pEngine, UUID sceneID, UUID objectID, UUID elementID, UUID componentID) {
			MonoObject* pEngineObject = MonoManager::Instance()->GetCoreLibManager()->GetEngine();
			void* args[] = {
				pEngineObject,
				&sceneID,
				&objectID,
				&elementID,
				&componentID
			};

			pDomain->InvokeMethod(pDownMethod, nullptr, args);
		};

		UIInteractionSystem::Instance()->OnElementUp_Callback =
		[pDomain, pUpMethod](Engine* pEngine, UUID sceneID, UUID objectID, UUID elementID, UUID componentID) {
			MonoObject* pEngineObject = MonoManager::Instance()->GetCoreLibManager()->GetEngine();
			void* args[] = {
				pEngineObject,
				&sceneID,
				&objectID,
				&elementID,
				&componentID
			};

			pDomain->InvokeMethod(pUpMethod, nullptr, args);
		};*/
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
