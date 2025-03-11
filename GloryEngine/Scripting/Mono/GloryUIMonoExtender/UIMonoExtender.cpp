#include "UIMonoExtender.h"
#include "UIComponentsCSAPI.h"
#include "UISceneCSAPI.h"
#include "MonoScriptedSystem.h"

#include <MonoManager.h>
#include <AssemblyDomain.h>
#include <CoreLibManager.h>
#include <UISystems.h>
#include <UIRendererModule.h>
#include <GloryMonoScipting.h>
#include <ScriptingExtender.h>
#include <Engine.h>
#include <Assembly.h>
#include <UIDocument.h>

namespace Glory
{
	UIMonoExtender::UIMonoExtender(const char* path) : m_pLibManager(new UILibManager()), m_Path(path)
	{
	}

	UIMonoExtender::~UIMonoExtender()
	{
		delete m_pLibManager;
		m_pLibManager = nullptr;
	}

	void UIMonoExtender::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		UIComponentsCSAPI::AddInternalCalls(internalCalls);
		UISceneCSAPI::AddInternalCalls(internalCalls);
	}

	void UIMonoExtender::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		pScriptingExtender->AddInternalLib(m_Path, "GloryEngine.UI.dll", m_pLibManager);
	}

	bool OnLoadExtra(const char* path, Module* pModule, Module* pRequiredModule)
	{
		GloryMonoScipting* pScripting = (GloryMonoScipting*)pRequiredModule;
		IScriptExtender* pScriptExtender = new UIMonoExtender(path);
		pScripting->GetScriptingExtender()->RegisterExtender(pScriptExtender);
		return true;
	}

	void UILibManager::Initialize(Engine* pEngine, Assembly* pAssembly)
	{
		UIComponentsCSAPI::SetEngine(pEngine);
		UISceneCSAPI::SetEngine(pEngine);
		AddMonoType("GloryEngine.UI.UIDocument", SerializedType::ST_Asset, ResourceTypes::GetHash<UIDocument>());

		UIRendererModule* pUIRenderer = pEngine->GetOptionalModule<UIRendererModule>();
		Utils::ECS::ComponentTypes::SetInstance(pUIRenderer->GetComponentTypes());

		AssemblyClass* pInteractionHandlerClass = pAssembly->GetClass("GloryEngine.UI", "InteractionHandler");
		MonoMethod* pHoverMethod = pInteractionHandlerClass->GetMethod(".::OnElementHover");
		MonoMethod* pUnhoverMethod = pInteractionHandlerClass->GetMethod(".::OnElementUnHover");
		MonoMethod* pDownMethod = pInteractionHandlerClass->GetMethod(".::OnElementDown");
		MonoMethod* pUpMethod = pInteractionHandlerClass->GetMethod(".::OnElementUp");

		AssemblyDomain* pDomain = pAssembly->GetDomain();

		UIInteractionSystem::Instance()->OnElementHover_Callback =
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
		};
	}

	void UILibManager::Cleanup(Engine*)
	{
	}
}
