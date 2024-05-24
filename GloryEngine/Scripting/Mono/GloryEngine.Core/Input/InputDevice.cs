using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Handle for an InputDevice
    /// </summary>
    public struct InputDevice
    {
        #region Props

        /// <summary>
        /// Name of the InputDevice
        /// </summary>
        public string Name => InputDevice_GetName(_deviceID);
        /// <summary>
        /// Type of the InputDevice
        /// </summary>
        public InputDeviceType DeviceType => InputDevice_GetInputDeviceType(_deviceID);
        /// <summary>
        /// The index of the InputDevice
        /// </summary>
        public uint DeviceID => InputDevice_GetDeviceID(_deviceID);
        /// <summary>
        /// The player that is currently using this device
        /// </summary>
        public int PlayerIndex => InputDevice_GetPlayerIndex(_deviceID);

        #endregion

        #region Fields

        private UInt64 _deviceID;

        #endregion

        #region Constructor

        internal InputDevice(UInt64 deviceID) { _deviceID = deviceID; }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string InputDevice_GetName(UInt64 deviceID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static InputDeviceType InputDevice_GetInputDeviceType(UInt64 deviceID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint InputDevice_GetDeviceID(UInt64 deviceID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static int InputDevice_GetPlayerIndex(UInt64 deviceID);

        #endregion
    }
}
