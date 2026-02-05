#include "RendererModule.h"
#include "PipelineData.h"

namespace Glory
{
	RendererModule::RendererModule()
	{
	}

	RendererModule::~RendererModule()
	{
	}

	const std::type_info& RendererModule::GetModuleType()
	{
		return typeid(RendererModule);
	}

	void RendererModule::OnBeginFrame()
	{
	}

	void RendererModule::OnEndFrame()
	{
	}

	void RendererModule::Initialize()
	{
	}

	void RendererModule::PostInitialize()
	{
		OnPostInitialize();
	}

	void RendererModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterAssetReference<PipelineData>("Lines Pipeline", 19);
	}

	void RendererModule::OnProcessData()
	{
	}
}
