#include "LocalizeModule.h"
#include "TextDatabase.h"

#include <Engine.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(LocalizeModule);

	LocalizeModule::LocalizeModule()
	{
	}

	LocalizeModule::~LocalizeModule()
	{
	}

	const std::type_info& LocalizeModule::GetModuleType()
	{
		return typeid(LocalizeModule);
	}

	void LocalizeModule::Initialize()
	{
		Utils::Reflect::Reflect::SetReflectInstance(&m_pEngine->Reflection());
		m_pEngine->GetResourceTypes().RegisterResource<TextDatabase>("");
	}

	void LocalizeModule::PostInitialize()
	{
	}

	void LocalizeModule::Update()
	{
	}

	void LocalizeModule::Cleanup()
	{
	}

	void LocalizeModule::LoadSettings(ModuleSettings& settings)
	{
	}
}
