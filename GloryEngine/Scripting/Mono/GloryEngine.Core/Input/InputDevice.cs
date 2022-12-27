using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public struct InputDevice
    {
        #region Props

        public string Name => InputDevice_GetName(_deviceID);
        public InputDeviceType DeviceType => InputDevice_GetInputDeviceType(_deviceID);
        public uint DeviceID => InputDevice_GetDeviceID(_deviceID);
        public int PlayerIndex => InputDevice_GetPlayerIndex(_deviceID);

        #endregion

        #region Fields

        private UInt64 _deviceID;

        #endregion

        #region Constructor

        public InputDevice(UInt64 deviceID) { _deviceID = deviceID; }

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
