#include "PlayerInput.h"
#include "Input.h"
#include "Debug.h"

namespace Glory
{
	PlayerInput::PlayerInput(InputModule* pInputModule, size_t playerIndex)
		: m_pInputModule(pInputModule), m_PlayerIndex(playerIndex), m_InputMode(InputMode::None.m_Name)
	{
	}

	PlayerInput::~PlayerInput()
	{
		m_InputData.clear();
	}

	std::string_view PlayerInput::InputMode()
	{
		return m_InputMode;
	}

	void PlayerInput::HandleInputEvent(InputEvent& e)
	{
		for (size_t i = 0; i < m_InputData.size(); ++i)
		{
			InputMap* pInputMap = m_InputData[i].m_InputMap;
			for (auto itor = pInputMap->m_Actions.begin(); itor != pInputMap->m_Actions.end(); itor++)
			{
				for (size_t j = 0; j < itor->second.m_Bindings.size(); ++j)
				{
					InputBinding& binding = itor->second.m_Bindings[j];
					if (itor->second.m_MappingType == InputMappingType::Bool)
					{
						if (binding.m_State == InputState::KeyPressed && e.State != InputState::KeyDown && e.State != InputState::KeyUp && binding.m_State != e.State) continue;
						else if (binding.m_State != InputState::KeyPressed && binding.m_State != e.State) continue;
					}

					if (!binding.m_KeyBinding.CheckEvent(e)) continue;
					std::filesystem::path path = "";
					path = path.append(pInputMap->m_Name).append(itor->second.m_Name);
					switch (itor->second.m_MappingType)
					{
					case InputMappingType::Bool:
						TriggerAction(m_InputData[i], itor->second, binding, e);
						break;

					case InputMappingType::Float:
						MapOnFloat(m_InputData[i], itor->second, binding, e);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	void PlayerInput::ClearActions()
	{
		for (size_t i = 0; i < m_InputData.size(); ++i)
		{
			m_InputData[i].m_TriggeredActions.clear();

			for (auto itor = m_InputData[i].m_AxisDeltas.begin(); itor != m_InputData[i].m_AxisDeltas.end(); ++itor)
			{
				itor->second = 0.0f;
			}

			for (size_t j = 0; j < m_InputData[i].m_ToClearValues.size(); ++j)
			{
				m_InputData[i].m_AxisValue[std::string(m_InputData[i].m_ToClearValues[i])] = 0.0f;
			}
			m_InputData[i].m_ToClearValues.clear();
		}
	}

	void PlayerInput::TriggerAction(PlayerInputData& inputData, InputAction& inputAction, InputBinding& inputBinding, InputEvent& e)
	{
		if (inputBinding.m_State == InputState::KeyPressed)
		{
			switch (e.State)
			{
			case InputState::KeyDown:
				if (inputData.m_PressedStates[inputBinding.m_Name]) return;
				inputData.m_PressedStates[inputBinding.m_Name] = true;
				break;

			case InputState::KeyUp:
				inputData.m_PressedStates[inputBinding.m_Name] = false;
				return; /* Don't trigger KeyPressed on KeyUp */
			default:
				/* Shouldnt happen but just in case */
				return;
			}
		}

		inputData.m_TriggeredActions.push_back(inputAction.m_Name);
		Debug::LogInfo("Input Action: " + inputAction.m_Name + " was triggered!");
		/* Forward to scripting? */
	}

	void PlayerInput::MapOnFloat(PlayerInputData& inputData, InputAction& inputAction, InputBinding& inputBinding, InputEvent& e)
	{
		inputData.m_AxisValue[inputAction.m_Name] = e.Value * inputBinding.m_Multiplier;
		inputData.m_AxisDeltas[inputAction.m_Name] = e.Delta * inputBinding.m_Multiplier;

		if (inputBinding.m_MapDeltaToValue)
		{
			inputData.m_AxisValue[inputAction.m_Name] = inputData.m_AxisDeltas[inputAction.m_Name];
			/* TODO: Add a ShouldClearOnNewFrame bool for clearing values */
			inputData.m_ToClearValues.push_back(inputAction.m_Name);
		}

		Debug::LogInfo("Input Action: " + inputAction.m_Name + " was triggered! Value: "
			+ std::to_string(inputData.m_AxisValue[inputAction.m_Name]) + " - Delta: " + std::to_string(inputData.m_AxisDeltas[inputAction.m_Name]));
	}

	PlayerInputData::PlayerInputData(InputMap* pInputMap) : m_InputMap(pInputMap)
	{
	}
}
