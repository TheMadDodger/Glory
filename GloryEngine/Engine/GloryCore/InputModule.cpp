#include "InputModule.h"

namespace Glory
{
	InputModule::InputModule()
		: m_Players()
	{
	}

	InputModule::~InputModule()
	{
	}

	const std::type_info& InputModule::GetModuleType()
	{
		return typeid(InputModule);
	}

	void InputModule::OnInput(InputEvent& event)
	{

	}

	size_t InputModule::AddPlayer()
	{
		const size_t index = m_Players.size();
		m_Players.push_back({ index });
		return index;
	}

	void InputModule::Initialize()
	{
		AddPlayer();

		/* TODO: Load runtime input mappings */
	}

	void InputModule::Cleanup()
	{
	}

	void InputModule::Update()
	{
	}
}
