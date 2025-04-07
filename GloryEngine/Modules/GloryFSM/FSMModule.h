#pragma once
#include <RendererModule.h>
#include <Glory.h>
#include <FileData.h>

#include <glm/glm.hpp>

#include <vector>

namespace Glory
{
	class FSMData;
	class FSMState;

	class FSMModule : public Module
	{
	public:
		GLORY_API FSMModule();
		GLORY_API virtual ~FSMModule();

		GLORY_API virtual const std::type_info& GetModuleType() override;

		GLORY_API UUID CreateFSMState(FSMData* pData);
		GLORY_API void DestroyFSMState(UUID id);
		GLORY_API void CleanupStates();

		GLORY_MODULE_VERSION_H(0,1,0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		std::vector<FSMState> m_States;
	};
}
