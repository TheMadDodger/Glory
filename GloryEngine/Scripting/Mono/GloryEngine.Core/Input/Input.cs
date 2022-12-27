using System;
using System.Collections.Generic;
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
        #region Props



        #endregion

        #region Methods

        public static bool GetInputDevice(InputDeviceType deviceType, uint index, out InputDevice device)
        {
            UInt64 deviceID = Input_GetDeviceID(deviceType, index);
            device = new InputDevice(deviceID);
            return deviceID != 0;
        }

        #endregion

        #region API Methods

        //[MethodImpl(MethodImplOptions.InternalCall)]
        //private extern static string Input_GetName(UInt64 objectID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 Input_GetDeviceID(InputDeviceType deviceType, uint index);

        #endregion
    }
}
