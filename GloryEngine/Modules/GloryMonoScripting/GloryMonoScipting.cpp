#include "GloryMonoScipting.h"
#include "CoreCSAPI.h"
#include "MathCSAPI.h"
#include "InputCSAPI.h"
#include "EntityCSAPI.h"
#include "MonoManager.h"
#include "ScriptedComponentSerializer.h"
#include "MonoComponents.h"
#include "MonoScriptedSystem.h"
#include "ScriptingExtender.h"

#include <SceneManager.h>
#include <PropertySerializer.h>
#include <Engine.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(GloryMonoScipting);

	GloryMonoScipting::GloryMonoScipting() : m_pMonoManager(new MonoManager(this)), m_pScriptingExtender(new ScriptingExtender())
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

	const std::type_info& GloryMonoScipting::GetModuleType()
	{
		return typeid(GloryMonoScipting);
	}

	ScriptingExtender* GloryMonoScipting::GetScriptingExtender()
	{
		return m_pScriptingExtender;
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
		m_pScriptingExtender->Initialize(this);
		m_pMonoManager->Initialize("./Modules/GloryMonoScripting/Dependencies");

		Reflect::SetReflectInstance(&m_pEngine->Reflection());

		Reflect::RegisterEnum<MonoLogLevel>();

		CoreCSAPI::SetEngine(m_pEngine);
		InputCSAPI::SetEngine(m_pEngine);
		EntityCSAPI::SetEngine(m_pEngine);

		m_pEngine->GetSceneManager()->RegisterComponent<MonoScriptComponent>();
		PropertySerializer::RegisterSerializer<ScriptedComponentSerailizer>();

		// Scripted
		m_pEngine->GetSceneManager()->ComponentTypesInstance()->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::OnAdd, MonoScriptedSystem::OnAdd);
		m_pEngine->GetSceneManager()->ComponentTypesInstance()->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::Update, MonoScriptedSystem::OnUpdate);
		m_pEngine->GetSceneManager()->ComponentTypesInstance()->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::Draw, MonoScriptedSystem::OnDraw);
		m_pEngine->GetSceneManager()->ComponentTypesInstance()->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::Start, MonoScriptedSystem::OnStart);
		m_pEngine->GetSceneManager()->ComponentTypesInstance()->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::Stop, MonoScriptedSystem::OnStop);
		m_pEngine->GetSceneManager()->ComponentTypesInstance()->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::OnValidate, MonoScriptedSystem::OnValidate);
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

	void GloryMonoScipting::GetInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		CoreCSAPI::AddInternalCalls(internalCalls);
		MathCSAPI::AddInternalCalls(internalCalls);
		InputCSAPI::AddInternalCalls(internalCalls);
		EntityCSAPI::GetInternallCalls(internalCalls);
	}

	void GloryMonoScipting::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		std::filesystem::path path = GetMetaData().Path();
		path.append("Scripting/csharp");
		pScriptingExtender->AddInternalLib(path.string(), "GloryEngine.Core.dll", m_pMonoManager->GetCoreLibManager());
	}
}