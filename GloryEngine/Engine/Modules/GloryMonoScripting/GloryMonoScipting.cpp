#include "GloryMonoScipting.h"
#include "CoreCSAPI.h"
#include "MathCSAPI.h"
#include "InputCSAPI.h"
#include "PhysicsCSAPI.h"
#include "MonoManager.h"

#include <Game.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(GloryMonoScipting);

	GloryMonoScipting::GloryMonoScipting() : ScriptingModuleTemplate("csharp"), m_pMonoManager(new MonoManager(this))
	{
	}

	GloryMonoScipting::~GloryMonoScipting()
	{
		delete m_pMonoManager;
		m_pMonoManager = nullptr;
	}

	MonoManager* GloryMonoScipting::GetMonoManager() const
	{
		return m_pMonoManager;
	}

	void GloryMonoScipting::LoadSettings(ModuleSettings& settings)
	{
		settings.PushGroup("Debugging");
		settings.RegisterValue<bool>("Enable Debugging", false);
		settings.RegisterEnumValue<MonoLogLevel>("MonoLogLevel", MonoLogLevel::error);
		settings.RegisterValue<std::string>("MonoDebuggingIP", "127.0.0.1");
		settings.RegisterValue<uint32_t>("MonoDebuggingPort", 55555);

		settings.PushGroup("Garbage Collection");
		settings.RegisterValue<bool>("Enable GC allocation logging", false);
		settings.RegisterValue<bool>("Auto Collect Garbage", true);
	}

	void GloryMonoScipting::Initialize()
	{
		m_pEngine->GetScriptingExtender()->RegisterExtender(this, this);
		m_pMonoManager->Initialize("./Modules/GloryMonoScripting/Dependencies");

		GloryReflect::Reflect::RegisterEnum<MonoLogLevel>();
	}

	void GloryMonoScipting::PostInitialize()
	{
		m_pMonoManager->InitialLoad();
	}

	void GloryMonoScipting::Cleanup()
	{
		m_pMonoManager->Cleanup();
	}

	void GloryMonoScipting::AddLib(const ScriptingLib& library)
	{
		m_pMonoManager->AddLib(library);
	}

	void GloryMonoScipting::Bind(const InternalCall& internalCall)
	{
		const std::string& scriptMethod = internalCall.ScriptMethod();
		mono_add_internal_call(scriptMethod.c_str(), internalCall.RedirectMethod());
	}

	std::string GloryMonoScipting::Language()
	{
		return ScriptingLanguage();
	}

	void GloryMonoScipting::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		CoreCSAPI::AddInternalCalls(internalCalls);
		MathCSAPI::AddInternalCalls(internalCalls);
		InputCSAPI::AddInternalCalls(internalCalls);
		PhysicsCSAPI::AddInternalCalls(internalCalls);
	}

	void GloryMonoScipting::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		pScriptingExtender->AddInternalLib("GloryEngine.Core.dll", m_pMonoManager->GetCoreLibManager());
	}
}
