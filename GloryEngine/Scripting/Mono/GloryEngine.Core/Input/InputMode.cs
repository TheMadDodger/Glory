using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public struct InputMode
    {
        #region Props

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
