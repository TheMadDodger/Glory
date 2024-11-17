#include "GloryMonoScipting.h"
#include "CoreCSAPI.h"
#include "MathCSAPI.h"
#include "InputCSAPI.h"
#include "EntityCSAPI.h"
#include "AudioCSAPI.h"
#include "MonoManager.h"
#include "ScriptedComponentSerializer.h"
#include "MonoComponents.h"
#include "MonoScriptedSystem.h"
#include "ScriptingExtender.h"
#include "MonoScriptLoader.h"

#include <Console.h>
#include <SceneManager.h>
#include <PropertySerializer.h>
#include <Engine.h>
#include <BinaryStream.h>

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
		m_pMonoManager->Initialize("./Modules/GloryMonoScripting/Dependencies");
		m_pScriptingExtender->Initialize(this);

		Reflect::SetReflectInstance(&m_pEngine->Reflection());

		Reflect::RegisterEnum<MonoLogLevel>();
		Reflect::RegisterType<ScriptTypeReference>();

		CoreCSAPI::SetEngine(m_pEngine);
		InputCSAPI::SetEngine(m_pEngine);
		EntityCSAPI::SetEngine(m_pEngine);
		AudioCSAPI::SetEngine(m_pEngine);

		m_pEngine->GetSceneManager()->RegisterComponent<MonoScriptComponent>();
		m_pEngine->GetSerializers().RegisterSerializer<ScriptedComponentSerailizer>();

		m_pEngine->GetResourceTypes().RegisterResource<MonoScript>(".cs");

		// Scripted
		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		pComponentTypes->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::Update, MonoScriptedSystem::OnUpdate);
		pComponentTypes->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::Draw, MonoScriptedSystem::OnDraw);
		pComponentTypes->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::Start, MonoScriptedSystem::OnStart);
		pComponentTypes->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::Stop, MonoScriptedSystem::OnStop);
		pComponentTypes->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::OnValidate, MonoScriptedSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::OnEnable, MonoScriptedSystem::OnEnable);
		pComponentTypes->RegisterInvokaction<MonoScriptComponent>(Glory::Utils::ECS::InvocationType::OnDisable, MonoScriptedSystem::OnDisable);

		m_pEngine->AddLoaderModule(new MonoScriptLoader());

		m_pEngine->GetConsole().RegisterCommand(new ConsoleCommand2<std::string, std::string>("loadAssembly", [this](std::string name, std::string file) {
			AddLib(ScriptingLib{ name, file, false, nullptr });
			return true;
		}));
	}

	void GloryMonoScipting::PostInitialize()
	{
		if (m_pEngine->HasData("Assemblies")) return;
		m_pMonoManager->InitialLoad();
	}

	void GloryMonoScipting::Cleanup()
	{
		m_pMonoManager->Cleanup();
	}

	void GloryMonoScipting::OnProcessData()
	{
		if (!m_pEngine->HasData("Assemblies")) return;
		std::vector<char>& data = m_pEngine->GetData("Assemblies");
		BinaryMemoryStream memoryStream{ data };
		BinaryStream& stream = memoryStream;
		stream.Read(m_AssembliesToLoad);

		const std::filesystem::path& rootPath = m_pEngine->RootPath();

		for (const std::filesystem::path& assembly : m_AssembliesToLoad)
		{
			std::filesystem::path path = rootPath;
			path.append(assembly.parent_path().string());
			AddLib(ScriptingLib{ assembly.filename().string(), path.string(), false, nullptr});
		}
		m_pMonoManager->InitialLoad();
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
		AudioCSAPI::GetInternallCalls(internalCalls);
	}

	void GloryMonoScipting::GetLibs(ScriptingExtender* pScriptingExtender)
	{
		std::filesystem::path path = GetMetaData().Path().parent_path();
		path.append("Scripting/csharp");
		pScriptingExtender->AddInternalLib(path.string(), "GloryEngine.Core.dll", m_pMonoManager->GetCoreLibManager());
	}
}