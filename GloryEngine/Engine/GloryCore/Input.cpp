#include "Input.h"
#include <filesystem>

namespace Glory
{
	InputMode InputMode::None{"None"};

	InputDevice::InputDevice(const char* name, const InputDeviceType deviceType, size_t deviceID)
		: m_Name(name), m_DeviceType(deviceType), m_DeviceID(deviceID), m_PlayerIndex(-1) {}

	InputMap::InputMap(const std::string name)
		: m_Name(name), m_Actions(std::map<std::string, InputAction>()) {}

	InputAction::InputAction(const std::string name, const InputMappingType mappingType, const AxisBlending axisBlending, const float blendSpeed)
		: m_Name(name), m_MappingType(mappingType), m_Blending(axisBlending), m_BlendSpeed(blendSpeed), m_Bindings(std::vector<InputBinding>()) {}

	KeyBinding::KeyBinding(const std::string bindingPath)
		: m_BindingPath(bindingPath), m_DeviceType(InputDeviceType(-1)), m_KeyID(0), m_IsAxis(false)
	{
		/* Parse binding path into values */
		if (m_BindingPath.empty())
		{
			m_DeviceType = InputDeviceType(-1);
			m_KeyID = 0;
			m_IsAxis = false;
			return;
		}

		std::filesystem::path bindingPathTemp{ m_BindingPath };
		for (auto itor = bindingPathTemp.begin(); itor != bindingPathTemp.end(); itor++)
		{
			const std::filesystem::path& subPath = *itor;
			if (GloryReflect::Enum<InputDeviceType>().FromString(subPath.string(), m_DeviceType)) continue;
			if (subPath.string() == "Axis")
			{
				m_IsAxis = true;
				continue;
			}
			else if (subPath.string() == "Axis")
			{
				m_IsAxis = false;
				continue;
			}

			KeyboardKey key{ KeyboardKey(-1) };
			if (GloryReflect::Enum<KeyboardKey>().FromString(subPath.string(), key))
			{
				m_KeyID = key;
				continue;
			}

			MouseButton mouseButton{ MouseButton(-1) };
			if (GloryReflect::Enum<MouseButton>().FromString(subPath.string(), mouseButton))
			{
				m_KeyID = mouseButton;
				continue;
			}

			MouseAxis mouseAxis{ MouseAxis(-1) };
			if (GloryReflect::Enum<MouseAxis>().FromString(subPath.string(), mouseAxis))
			{
				m_KeyID = mouseAxis;
				continue;
			}
		}
	}

	KeyBinding::KeyBinding(const KeyBinding& other) noexcept
		: m_BindingPath(other.m_BindingPath), m_DeviceType(other.m_DeviceType), m_KeyID(other.m_KeyID), m_IsAxis(other.m_IsAxis) {}

	KeyBinding& KeyBinding::operator=(const KeyBinding&& other) noexcept
	{
		m_BindingPath = other.m_BindingPath;
		m_DeviceType = other.m_DeviceType;
		m_KeyID = other.m_KeyID;
		m_IsAxis = other.m_IsAxis;
		return *this;
	}

	bool KeyBinding::CheckEvent(InputEvent& e)
	{
		if (m_IsAxis && e.State != InputState::Axis) return false;
		return e.InputDeviceType == m_DeviceType && m_KeyID == e.KeyID;
	}

	InputBinding::InputBinding(const std::string name, const InputState inputState, const float multiplier, const bool mapDeltaToValue, const KeyBinding keybinding)
		: m_Name(name), m_State(inputState), m_Multiplier(multiplier), m_MapDeltaToValue(mapDeltaToValue), m_KeyBinding(keybinding) {}

	InputMode::InputMode(const std::string name)
		: m_Name(name), m_DeviceTypes(std::vector<InputDeviceType>()) {}

	InputMode::InputMode(InputMode&& other) noexcept
		: m_Name(std::move(other.m_Name)), m_DeviceTypes(std::move(other.m_DeviceTypes)) {}
};