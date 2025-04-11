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
	struct FSMNode;

	class FSMModule : public Module
	{
	public:
		GLORY_API FSMModule();
		GLORY_API virtual ~FSMModule();

		GLORY_API virtual const std::type_info& GetModuleType() override;

		GLORY_API UUID CreateFSMState(FSMData* pData);
		GLORY_API FSMState* GetFSMState(UUID id);
		GLORY_API void DestroyFSMState(UUID id);
		GLORY_API void CleanupStates();

		GLORY_API size_t FSMInstanceCount() const;
		GLORY_API const FSMState& FSMInstance(size_t index) const;
		GLORY_API const FSMState* FSMInstance(UUID instanceID) const;

		GLORY_MODULE_VERSION_H(1,0,0);

		std::function<void(const FSMState&, const FSMNode&)> EntryCallback = NULL;
		std::function<void(const FSMState&, const FSMNode&)> ExitCallback = NULL;

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
