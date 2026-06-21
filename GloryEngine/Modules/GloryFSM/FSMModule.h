#pragma once
#include "fsm_visibility.h"

#include <Version.h>
#include <Module.h>

#include <vector>
#include <functional>

namespace Glory
{
	class FSMData;
	class FSMState;
	struct FSMNode;

	class FSMModule : public Module
	{
	public:
		GLORY_FSM_API FSMModule();
		GLORY_FSM_API virtual ~FSMModule();

		GLORY_FSM_API virtual const std::type_info& GetModuleType() override;

		GLORY_FSM_API UUID CreateFSMState(FSMData* pData);
		GLORY_FSM_API FSMState* GetFSMState(UUID id);
		GLORY_FSM_API void DestroyFSMState(UUID id);
		GLORY_FSM_API void CleanupStates();

		GLORY_FSM_API size_t FSMInstanceCount() const;
		GLORY_FSM_API const FSMState& FSMInstance(size_t index) const;
		GLORY_FSM_API const FSMState* FSMInstance(UUID instanceID) const;

		GLORY_MODULE_VERSION_H(1, 1, 0);

		std::function<void(const FSMState&, const FSMNode&)> EntryCallback = NULL;
		std::function<void(const FSMState&, const FSMNode&)> ExitCallback = NULL;

	private:
		virtual void RegisterTypes() override;
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Cleanup() override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	private:
		std::vector<FSMState> m_States;
	};
}
