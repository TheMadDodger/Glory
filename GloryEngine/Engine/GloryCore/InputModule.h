#pragma once
#include "Module.h"
#include "Input.h"
#include <glm/glm.hpp>

namespace Glory
{
	class InputModule : public Module
	{
	public:
		InputModule();
		virtual ~InputModule();

		virtual const std::type_info& GetModuleType() override;

		void OnInput(InputEvent& event);

		size_t AddPlayer();

	protected:
		virtual void OnInitialize() {};
		virtual void OnCleanup() {};
		virtual void OnUpdate() {};

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void Update() override;

	private:
		std::vector<PlayerInput> m_Players;
		std::map<std::string, std::map<InputMode, InputMap>> m_InputMaps;
	};
}
