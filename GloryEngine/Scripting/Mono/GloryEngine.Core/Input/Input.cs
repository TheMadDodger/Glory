using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// A type of InputDevice
    /// </summary>
    public enum InputDeviceType
    {
        /// <summary>
        /// Kayboard
        /// </summary>
        Keyboard,
        /// <summary>
        /// Mouse
        /// </summary>
        Mouse,
        /// <summary>
        /// Controller/pad
        /// </summary>
        Gamepad
    }

    /// <summary>
    /// Global class for handling input
    /// </summary>
    public static class Input
    {
        #region Methods

        /// <summary>
        /// Aqcuire a connected input device
        /// </summary>
        /// <param name="deviceType">The type of the device</param>
        /// <param name="index">The number of the device</param>
        /// <param name="device">The out InputDevice handle</param>
        /// <returns>True if the device was found, false if the device does not exist</returns>
        public static bool GetInputDevice(InputDeviceType deviceType, uint index, out InputDevice device)
        {
            UInt64 deviceID = Input_GetDeviceID(deviceType, index);
            device = new InputDevice(deviceID);
            return deviceID != 0;
        }

        /// <summary>
        /// Get an InputMode handle
        /// </summary>
        /// <param name="name">The name of the InputMode</param>
        /// <returns>The InputMode handle that matches the name, or an invalid handle if not found</returns>
        public static InputMode GetInputMode(string name)
        {
            name = Input_GetInputMode(name);
            return name != null ? new InputMode(name) : new InputMode("None");
        }

        /// <summary>
        /// Get the float value of an input axis
        /// </summary>
        /// <param name="playerIndex">The player to get the input data from</param>
        /// <param name="inputMap">The name of the InputMap to get the action from</param>
        /// <param name="actionName">The name of the action in the InputMap to get the axis value from</param>
        /// <returns>Current axis value, if the axis is not found 0 is returned by default</returns>
        public static float GetAxis(uint playerIndex, string inputMap, string actionName) => Input_GetAxis(playerIndex, inputMap, actionName);
        /// <summary>
        /// Get the amount the axis has changed since the last fame
        /// </summary>
        /// <param name="playerIndex">The player to get the input data from</param>
        /// <param name="inputMap">The name of the InputMap to get the action from</param>
        /// <param name="actionName">The name of the action in the InputMap to get the axis value from</param>
        /// <returns>Current axis delta value, if the axis is not found 0 is returned by default</returns>
        public static float GetAxisDelta(uint playerIndex, string inputMap, string actionName) => Input_GetAxisDelta(playerIndex, inputMap, actionName);
        /// <summary>
        /// Get the current cursor position for a player
        /// </summary>
        /// <param name="playerIndex">The player to get the cursor from</param>
        /// <returns>Current cursor position</returns>
        public static Vector2 GetCursorPos(uint playerIndex) => Input_GetCursorPos(playerIndex);
        /// <summary>
        /// Check wether an action was triggered this frame
        /// </summary>
        /// <param name="playerIndex">The player to get the input data from</param>
        /// <param name="inputMap">The name of the InputMap to get the action from</param>
        /// <param name="actionName">The name of the action in the InputMap to test</param>
        /// <returns>True if the action was triggered this frame, if the action is not found false is returned by default</returns>
        public static bool IsActionTriggered(uint playerIndex, string inputMap, string actionName) => Input_IsActionTriggered(playerIndex, inputMap, actionName);
        /// <summary>
        /// Change the InputMode of a player
        /// This clears the players input data so if the specified InputMode is not found
        /// the players InputMode will be unknown and will stop receiving any inputs
        /// </summary>
        /// <param name="playerIndex">The player to change the InputMode on</param>
        /// <param name="inputMode">The name of the InputMode to change to</param>
        public static void SetPlayerInputMode(uint playerIndex, string inputMode) => Input_SetPlayerInputMode(playerIndex, inputMode);
        /// <summary>
        /// Add a new player to session
        /// </summary>
        /// <returns>The index of the new player</returns>
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
        private extern static Vector2 Input_GetCursorPos(uint playerIndex);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool Input_IsActionTriggered(uint playerIndex, string inputMap, string actionName);

        #endregion
    }
}
