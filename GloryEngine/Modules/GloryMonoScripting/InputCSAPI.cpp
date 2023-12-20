#include "InputCSAPI.h"
#include "GloryMonoScipting.h"
#include "MonoManager.h"

#include <Engine.h>
#include <InputModule.h>

namespace Glory
{
	Engine* Input_EngineInstance;

#pragma region Input Device

	MonoString* InputDevice_GetName(uint64_t deviceID)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		InputDevice* pDevice = pInputModule->GetInputDevice(deviceID);
		if (!pDevice) return mono_string_new(mono_domain_get(), "Unknown device");
		return mono_string_new(mono_domain_get(), pDevice->m_Name);
	}

	InputDeviceType InputDevice_GetInputDeviceType(uint64_t deviceID)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		InputDevice* pDevice = pInputModule->GetInputDevice(deviceID);
		if (!pDevice) return InputDeviceType(-1);
		return pDevice->m_DeviceType;
	}

	size_t InputDevice_GetDeviceID(uint64_t deviceID)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		InputDevice* pDevice = pInputModule->GetInputDevice(deviceID);
		if (!pDevice) return 0;
		return pDevice->m_DeviceID;
	}

	int InputDevice_GetPlayerIndex(uint64_t deviceID)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		InputDevice* pDevice = pInputModule->GetInputDevice(deviceID);
		if (!pDevice) return -1;
		return pDevice->m_PlayerIndex;
	}

#pragma endregion

#pragma region Input Module

	uint64_t Input_GetDeviceID(InputDeviceType deviceType, size_t index)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		return pInputModule->GetDeviceUUID(InputDeviceType(deviceType), index);
	}

	MonoString* Input_GetInputMode(MonoString* name)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		InputMode* inputMode = pInputModule->GetInputMode(mono_string_to_utf8(name));
		return inputMode ? name : nullptr;
	}

	size_t Input_AddPlayer()
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		return pInputModule->AddPlayer();
	}

	void Input_RemovePlayer(size_t playerIndex)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		pInputModule->RemovePlayer(playerIndex);
	}

	void Input_SetPlayerInputMode(size_t playerIndex, MonoString* inputMode)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		pInputModule->SetPlayerInputMode(playerIndex, mono_string_to_utf8(inputMode));
	}

	float Input_GetAxis(size_t playerIndex, MonoString* inputMap, MonoString* actionName)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		return pInputModule->GetAxis(playerIndex, mono_string_to_utf8(inputMap), mono_string_to_utf8(actionName));
	}

	float Input_GetAxisDelta(size_t playerIndex, MonoString* inputMap, MonoString* actionName)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		return pInputModule->GetAxisDelta(playerIndex, mono_string_to_utf8(inputMap), mono_string_to_utf8(actionName));
	}

	bool Input_IsActionTriggered(size_t playerIndex, MonoString* inputMap, MonoString* actionName)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		return pInputModule->GetBool(playerIndex, mono_string_to_utf8(inputMap), mono_string_to_utf8(actionName));
	}

#pragma endregion

#pragma region Input Modes

	size_t InputMode_GetDeviceCount(MonoString* name)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		InputMode* inputMode = pInputModule->GetInputMode(mono_string_to_utf8(name));
		return inputMode ? inputMode->m_DeviceTypes.size() : 0;
	}

	InputDeviceType InputMode_GetInputDeviceType(MonoString* name, size_t index)
	{
		InputModule* pInputModule = Input_EngineInstance->GetMainModule<InputModule>();
		InputMode* inputMode = pInputModule->GetInputMode(mono_string_to_utf8(name));
		return (inputMode && inputMode->m_DeviceTypes.size() < index) ? inputMode->m_DeviceTypes[index] : InputDeviceType(-1);
	}

#pragma endregion

#pragma region Binding

	void InputCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		/* Input Module */
		BIND("GloryEngine.Input::Input_GetDeviceID", Input_GetDeviceID);
		BIND("GloryEngine.Input::Input_GetInputMode", Input_GetInputMode);
		BIND("GloryEngine.Input::Input_AddPlayer", Input_AddPlayer);
		BIND("GloryEngine.Input::Input_RemovePlayer", Input_RemovePlayer);
		BIND("GloryEngine.Input::Input_SetPlayerInputMode", Input_SetPlayerInputMode);
		BIND("GloryEngine.Input::Input_GetAxis", Input_GetAxis);
		BIND("GloryEngine.Input::Input_GetAxisDelta", Input_GetAxisDelta);
		BIND("GloryEngine.Input::Input_IsActionTriggered", Input_IsActionTriggered);

		/* Input Device */
		BIND("GloryEngine.InputDevice::InputDevice_GetName", InputDevice_GetName);
		BIND("GloryEngine.InputDevice::InputDevice_GetInputDeviceType", InputDevice_GetInputDeviceType);
		BIND("GloryEngine.InputDevice::InputDevice_GetDeviceID", InputDevice_GetDeviceID);
		BIND("GloryEngine.InputDevice::InputDevice_GetPlayerIndex", InputDevice_GetPlayerIndex);

		/* Input Modes */
		BIND("GloryEngine.InputDevice::InputMode_GetDeviceCount", InputMode_GetDeviceCount);
		BIND("GloryEngine.InputDevice::InputMode_GetInputDeviceType", InputMode_GetInputDeviceType);
	}

	void InputCSAPI::SetEngine(Engine* pEngine)
	{
		Input_EngineInstance = pEngine;
	}

#pragma endregion
}
