#include "FSMModule.h"

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

	void FSMModule::Initialize()
	{
	}

	void FSMModule::PostInitialize()
	{
	}

	void FSMModule::Update()
	{
	}

	void FSMModule::Cleanup()
	{
	}

	void FSMModule::LoadSettings(ModuleSettings& settings)
	{
	}
}
