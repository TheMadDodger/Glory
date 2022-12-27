#include "Input.h"

namespace Glory
{
	InputMode InputMode::None{"None"};

	InputDevice::InputDevice(const char* name, const InputDeviceType deviceType, size_t deviceID)
		: m_Name(name), m_DeviceType(deviceType), m_DeviceID(deviceID), m_PlayerIndex(-1) {}

	InputMap::InputMap(const std::string name)
		: m_Name(name), m_Actions(std::map<std::string, InputAction>()) {}

	InputAction::InputAction(const std::string name, const InputMappingType mappingType)
		: m_Name(name), m_MappingType(mappingType), m_Bindings(std::vector<InputBinding>()) {}

	KeyBinding::KeyBinding(const std::string bindingPath)
		: m_BindingPath(bindingPath)
	{
		/* Parse binding path into values */

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

	InputBinding::InputBinding(const std::string name, const InputMappingType mappingType, const float multiplier, const KeyBinding keybinding)
		: m_Name(name), m_MappingType(mappingType), m_Multiplier(multiplier), m_KeyBinding(keybinding) {}

	InputMode::InputMode(const std::string name)
		: m_Name(name), m_DeviceTypes(std::vector<InputDeviceType>()) {}

	InputMode::InputMode(InputMode&& other) noexcept
		: m_Name(std::move(other.m_Name)), m_DeviceTypes(std::move(other.m_DeviceTypes)) {}
}