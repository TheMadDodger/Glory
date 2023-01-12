using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// A handle for an InputMode
    /// </summary>
    public struct InputMode
    {
        #region Props

        /// <summary>
        /// Number of devices that this InputMode requires
        /// </summary>
        public uint DeviceCount => InputMode_GetDeviceCount(_name);

        #endregion

        #region Fields

        private string _name;

        #endregion

        #region Constructor

        public InputMode(string name)
        {
            _name = name;
        }

        #endregion

        #region Methods

        /// <summary>
        /// Get the device type that this InputMode needs
        /// </summary>
        /// <param name="index">Index of the device type</param>
        /// <returns>The device type this InputMode needs at the specified index</returns>
        public InputDeviceType GetInputDeviceType(uint index) => InputMode_GetInputDeviceType(_name, index);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint InputMode_GetDeviceCount(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static InputDeviceType InputMode_GetInputDeviceType(string name, uint index);

        #endregion
    }
}
