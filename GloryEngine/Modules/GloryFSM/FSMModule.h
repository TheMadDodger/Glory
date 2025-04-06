#pragma once
#include <RendererModule.h>
#include <Glory.h>
#include <FileData.h>

#include <glm/glm.hpp>

namespace Glory
{
	class FSMModule : public Module
	{
	public:
		GLORY_API FSMModule();
		GLORY_API virtual ~FSMModule();

		GLORY_API virtual const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

		virtual void LoadSettings(ModuleSettings& settings) override;
	};
}
