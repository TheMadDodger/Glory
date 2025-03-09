#include "UIMonoExtender.h"
#include "UIComponentsCSAPI.h"
#include "UISceneCSAPI.h"
#include "MonoScriptedSystem.h"

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

	void UILibManager::Initialize(Engine* pEngine, Assembly*)
	{
		UIComponentsCSAPI::SetEngine(pEngine);
		UISceneCSAPI::SetEngine(pEngine);
		AddMonoType("GloryEngine.UI.UIDocument", SerializedType::ST_Asset, ResourceTypes::GetHash<UIDocument>());

		UIRendererModule* pUIRenderer = pEngine->GetOptionalModule<UIRendererModule>();
		Utils::ECS::ComponentTypes::SetInstance(pUIRenderer->GetComponentTypes());
	}

	void UILibManager::Cleanup(Engine*)
	{
	}
}
