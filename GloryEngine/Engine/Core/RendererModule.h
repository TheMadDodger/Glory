#pragma once
#include "Module.h"

namespace Glory
{
	class RendererModule : public Module
	{
	public:
		RendererModule();
		virtual ~RendererModule();

		virtual const std::type_info& GetModuleType() override;

		virtual void OnBeginFrame() override;
		virtual void OnEndFrame() override;

	protected:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Cleanup() = 0;

		virtual void OnPostInitialize() {};

		virtual void LoadSettings(ModuleSettings& settings) override;
	};
}
