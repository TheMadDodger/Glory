#pragma once
#include "KeyEnums.h"
#include <glm/vec2.hpp>

REFLECTABLE_ENUM_NS(Glory, InputDeviceType, Keyboard, Mouse, Gamepad)
REFLECTABLE_ENUM_NS(Glory, InputMappingType, Bool, Float)
REFLECTABLE_ENUM_NS(Glory, InputState, KeyPressed, KeyDown, KeyUp, Axis)
REFLECTABLE_ENUM_NS(Glory, AxisBlending, Jump, Lerp, SLerp)

namespace Glory
{
	struct InputDevice
	{
		InputDevice(const char* name, const InputDeviceType deviceType, size_t deviceID);

		const char* m_Name;
		const InputDeviceType m_DeviceType;
		const size_t m_DeviceID;
		int m_PlayerIndex;
	};

	struct InputMode
	{
		InputMode(const std::string name);
		InputMode(InputMode&& other) noexcept;
		const std::string m_Name;
		std::vector<InputDeviceType> m_DeviceTypes;

		static InputMode None;
	};

	struct InputEvent
	{
		InputDeviceType InputDeviceType;
		size_t SourceDeviceID;
		size_t KeyID;
		InputState State;
		float Value;
		float Delta;
		KeyboardMod KeyMods;
	};

	struct CursorEvent
	{
		enum Type
		{
			Motion,
			Button,
			Scroll
		};

		Type Type;
		InputDeviceType InputDeviceType;
		size_t SourceDeviceID;
		glm::vec2 Cursor;
		bool IsDelta;
		bool IsDown;
	};

	struct TextEvent
	{
		InputDeviceType InputDeviceType;
		char Character;
	};

	struct KeyBindingCompact
	{
		InputDeviceType m_DeviceType;
		size_t m_KeyID;
		bool m_IsAxis;
	};

	struct KeyBinding
	{
		KeyBinding(const std::string bindingPath, const KeyBindingCompact& compact);
		KeyBinding(const std::string bindingPath);
		KeyBinding(const KeyBinding& other) noexcept;

		KeyBinding& operator=(const KeyBinding&& other) noexcept;

		bool CheckEvent(InputEvent& e);

		KeyBindingCompact Compact();

		std::string m_BindingPath;
		InputDeviceType m_DeviceType;
		size_t m_KeyID;
		bool m_IsAxis;
	};

	struct InputBinding
	{
		InputBinding(const std::string name, const InputState inputState, const float multiplier, const bool mapDeltaToValue, const KeyBinding keybinding);

		const std::string m_Name;
		const InputState m_State;
		const float m_Multiplier;
		const bool m_MapDeltaToValue;
		KeyBinding m_KeyBinding;

	private:
		//InputBinding(const InputBinding& other) = delete;
		//InputBinding& operator=(const InputBinding&& other) = delete;
	};

	struct InputAction
	{
		InputAction(const std::string, const InputMappingType mappingType, const AxisBlending axisBlending, const float blendSpeed);

		const std::string m_Name;
		const InputMappingType m_MappingType;
		const AxisBlending m_Blending;
		const float m_BlendSpeed;
		std::vector<InputBinding> m_Bindings;

	private:
		//InputAction(const InputAction& other) = delete;
		//InputAction& operator=(const InputAction&& other) = delete;
	};

	struct InputMap
	{
		InputMap(const std::string name);

		const std::string m_Name;
		std::map<std::string, InputAction> m_Actions;

	private:
		//InputMap(const InputMap& other) = delete;
		//InputMap& operator=(const InputMap&& other) = delete;
	};
}
