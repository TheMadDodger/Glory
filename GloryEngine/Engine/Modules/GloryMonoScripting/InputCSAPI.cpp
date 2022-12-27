#include "InputCSAPI.h"
#include "GloryMonoScipting.h"
#include "MonoManager.h"
#include <InputModule.h>

#define INPUT_MODULE Game::GetGame().GetEngine()->GetInputModule();

namespace Glory
{
#pragma region Input Device

	MonoString* InputDevice_GetName(uint64_t deviceID)
	{
		InputModule* pInputModule = INPUT_MODULE;
		InputDevice* pDevice = pInputModule->GetInputDevice(deviceID);
		if (!pDevice) return mono_string_new(MonoManager::GetDomain(), "Unknown device");
		return mono_string_new(MonoManager::GetDomain(), pDevice->m_Name);
	}

	InputDeviceType InputDevice_GetInputDeviceType(uint64_t deviceID)
	{
		InputModule* pInputModule = INPUT_MODULE;
		InputDevice* pDevice = pInputModule->GetInputDevice(deviceID);
		if (!pDevice) return InputDeviceType(-1);
		return pDevice->m_DeviceType;
	}

	size_t InputDevice_GetDeviceID(uint64_t deviceID)
	{
		InputModule* pInputModule = INPUT_MODULE;
		InputDevice* pDevice = pInputModule->GetInputDevice(deviceID);
		if (!pDevice) return 0;
		return pDevice->m_DeviceID;
	}

	int InputDevice_GetPlayerIndex(uint64_t deviceID)
	{
		InputModule* pInputModule = INPUT_MODULE;
		InputDevice* pDevice = pInputModule->GetInputDevice(deviceID);
		if (!pDevice) return -1;
		return pDevice->m_PlayerIndex;
	}

#pragma endregion

#pragma region Input Module

	uint64_t Input_GetDeviceID(InputDeviceType deviceType, size_t index)
	{
		InputModule* pInputModule = INPUT_MODULE;
		return pInputModule->GetDeviceUUID(InputDeviceType(deviceType), index);
	}

	void Input_SetInputMode()
	{
		InputModule* pInputModule = INPUT_MODULE;
	}

#pragma endregion

#pragma region Binding

	void InputCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		/* Input Module */
		BIND("GloryEngine.Input::Input_GetDeviceID", Input_GetDeviceID);

		/* Input Device */
		BIND("GloryEngine.InputDevice::InputDevice_GetName", InputDevice_GetName);
		BIND("GloryEngine.InputDevice::InputDevice_GetInputDeviceType", InputDevice_GetInputDeviceType);
		BIND("GloryEngine.InputDevice::InputDevice_GetDeviceID", InputDevice_GetDeviceID);
		BIND("GloryEngine.InputDevice::InputDevice_GetPlayerIndex", InputDevice_GetPlayerIndex);
	}

#pragma endregion
}
