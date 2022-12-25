#pragma once
#include <Reflection.h>
#include "KeyEnums.h"
#include "PlayerInput.h"
#include "InputMap.h"

REFLECTABLE_ENUM_NS(Glory, InputMappingType, Bool, Float)
REFLECTABLE_ENUM_NS(Glory, InputDeviceType, Keyboard, Mouse, Gamepad)

namespace Glory
{
	struct InputDevice
	{

	};

	struct InputMode
	{
		InputMode(const char* name) : m_Name(name) {}

		const char* m_Name;
	};

	struct InputModes
	{
		static InputMode None;
	};

	enum class InputState
	{
		KeyUp,
		KeyDown,
		Axis,
	};

	struct InputEvent
	{
		InputDeviceType InputDeviceType;
		size_t SourceDeviceID;
		size_t KeyID;
		InputState State;
		float Value;
		float Delta;
	};
}
