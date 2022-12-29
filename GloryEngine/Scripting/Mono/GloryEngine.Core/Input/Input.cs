using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public enum InputDeviceType
    {
        Keyboard,
        Mouse,
        Gamepad
    }

    public static class Input
    {
        #region Methods

        public static bool GetInputDevice(InputDeviceType deviceType, uint index, out InputDevice device)
        {
            UInt64 deviceID = Input_GetDeviceID(deviceType, index);
            device = new InputDevice(deviceID);
            return deviceID != 0;
        }

        public static InputMode GetInputMode(string name)
        {
            name = Input_GetInputMode(name);
            return name != null ? new InputMode(name) : new InputMode("None");
        }

        public static float GetAxis(uint playerIndex, string inputMap, string actionName) => Input_GetAxis(playerIndex, inputMap, actionName);
        public static float GetAxisDelta(uint playerIndex, string inputMap, string actionName) => Input_GetAxisDelta(playerIndex, inputMap, actionName);
        public static bool IsActionTriggered(uint playerIndex, string inputMap, string actionName) => Input_IsActionTriggered(playerIndex, inputMap, actionName);

        public static void SetPlayerInputMode(uint playerIndex, string inputMode) => Input_SetPlayerInputMode(playerIndex, inputMode);

        public static uint AddPlayer() => Input_AddPlayer();

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 Input_GetDeviceID(InputDeviceType deviceType, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string Input_GetInputMode(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint Input_AddPlayer();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Input_SetPlayerInputMode(uint playerIndex, string inputMode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Input_GetAxis(uint playerIndex, string inputMap, string actionName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Input_GetAxisDelta(uint playerIndex, string inputMap, string actionName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool Input_IsActionTriggered(uint playerIndex, string inputMap, string actionName);

        #endregion
    }
}
