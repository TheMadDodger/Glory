#include "PlayerInput.h"
#include "InputModule.h"
#include "Debug.h"
#include "GameTime.h"

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
				const std::string& toClear = m_InputData[i].m_ToClearValues[j];
				m_InputData[i].m_AxisValueRight[toClear] = 0.0f;
				m_InputData[i].m_AxisValueLeft[toClear] = 0.0f;
			}
			m_InputData[i].m_ToClearValues.clear();
		}
	}

	const float PlayerInput::GetAxis(const std::string& inputMap, const std::string& actionName)
	{
		for (size_t i = 0; i < m_InputData.size(); i++)
		{
			if (m_InputData[i].m_InputMap->m_Name != inputMap) continue;
			return m_InputData[i].m_AxisValueRight[actionName] + m_InputData[i].m_AxisValueLeft[actionName];
		}
		return 0.0f;
	}

	const float PlayerInput::GetAxisDelta(const std::string& inputMap, const std::string& actionName)
	{
		for (size_t i = 0; i < m_InputData.size(); i++)
		{
			if (m_InputData[i].m_InputMap->m_Name != inputMap) continue;
			return m_InputData[i].m_AxisDeltas[actionName];
		}
		return 0.0f;
	}

	const bool PlayerInput::GetBool(const std::string& inputMap, const std::string& actionName)
	{
		for (size_t i = 0; i < m_InputData.size(); i++)
		{
			if (m_InputData[i].m_InputMap->m_Name != inputMap) continue;
			return std::find(m_InputData[i].m_TriggeredActions.begin(), m_InputData[i].m_TriggeredActions.end(), actionName)
				!= m_InputData[i].m_TriggeredActions.end();
		}
		return false;
	}

	void PlayerInput::Unbind()
	{
		m_InputData.clear();
		for (size_t i = 0; i < m_ClaimedDevices.size(); i++)
		{
			m_pInputModule->FreeDevice(m_ClaimedDevices[i]);
		}
		m_ClaimedDevices.clear();
		m_ClaimedDeviceTypes.clear();

		m_InputMode = InputMode::None.m_Name;
	}

	void PlayerInput::Update()
	{
		for (size_t i = 0; i < m_InputData.size(); i++)
		{
			for (auto itor = m_InputData[i].m_AxisDesiredValueLeft.begin(); itor != m_InputData[i].m_AxisDesiredValueLeft.end(); ++itor)
			{
				InputAction& inputAction = m_InputData[i].m_InputMap->m_Actions.at(itor->first);
				const float lFrac = std::clamp(inputAction.m_BlendSpeed * Time::GetDeltaTime<float, std::ratio<1, 1>>(), 0.0f, 0.1f);
				switch (inputAction.m_Blending)
				{
				case AxisBlending::Jump:
					m_InputData[i].m_AxisValueLeft[itor->first] = m_InputData[i].m_AxisDesiredValueLeft[itor->first];
					break;
				case AxisBlending::Lerp:
				{
					m_InputData[i].m_AxisValueLeft[itor->first] = Lerp(m_InputData[i].m_AxisValueLeft[itor->first], m_InputData[i].m_AxisDesiredValueLeft[itor->first], lFrac);
					break;
				}
				case AxisBlending::SLerp:

					break;
				default:
					break;
				}
			}

			for (auto itor = m_InputData[i].m_AxisDesiredValueRight.begin(); itor != m_InputData[i].m_AxisDesiredValueRight.end(); ++itor)
			{
				InputAction& inputAction = m_InputData[i].m_InputMap->m_Actions.at(itor->first);
				const float lFrac = std::clamp(inputAction.m_BlendSpeed * Time::GetDeltaTime<float, std::ratio<1, 1>>(), 0.0f, 0.1f);
				switch (inputAction.m_Blending)
				{
				case AxisBlending::Jump:
					m_InputData[i].m_AxisValueRight[itor->first] = m_InputData[i].m_AxisDesiredValueRight[itor->first];
					break;
				case AxisBlending::Lerp:
				{
					m_InputData[i].m_AxisValueRight[itor->first] = Lerp(m_InputData[i].m_AxisValueRight[itor->first], m_InputData[i].m_AxisDesiredValueRight[itor->first], lFrac);
					break;
				}
				case AxisBlending::SLerp:

					break;
				default:
					break;
				}
			}
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
		/* TODO: Forward to scripting for triggering input event */
	}

	void PlayerInput::MapOnFloat(PlayerInputData& inputData, InputAction& inputAction, InputBinding& inputBinding, InputEvent& e)
	{
		/* TODO: Split axis into left and right channels, then make GetAxis blend between the 2? */
		const float value = e.Value * inputBinding.m_Multiplier;
		if (inputBinding.m_Multiplier > 0.0f) inputData.m_AxisDesiredValueRight[inputAction.m_Name] = value;
		else if (inputBinding.m_Multiplier < 0.0f) inputData.m_AxisDesiredValueLeft[inputAction.m_Name] = value;
		inputData.m_AxisDeltas[inputAction.m_Name] = e.Delta * std::abs(inputBinding.m_Multiplier);

		if (inputBinding.m_MapDeltaToValue)
		{
			inputData.m_AxisDesiredValueLeft[inputAction.m_Name] = inputData.m_AxisDeltas[inputAction.m_Name];
			inputData.m_AxisDesiredValueRight[inputAction.m_Name] = inputData.m_AxisDeltas[inputAction.m_Name];
			/* TODO: Add a ShouldClearOnNewFrame bool for clearing values */
			inputData.m_ToClearValues.push_back(inputAction.m_Name);
		}

		/* TODO: Forward to scripting for triggering input event */
	}

	float PlayerInput::Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	PlayerInputData::PlayerInputData(InputMap* pInputMap) : m_InputMap(pInputMap)
	{
	}
}
