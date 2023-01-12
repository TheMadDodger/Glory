using System;

namespace GloryEngine
{
    /// <summary>
    /// A handle for a camera
    /// </summary>
    public struct Camera
    {
        #region Props

        public UInt64 CameraID => _cameraID;

        #endregion

        #region Fields

        private UInt64 _cameraID;

        #endregion

        #region Constructor

        public Camera(UInt64 id)
        {
            _cameraID = id;
        }

		#endregion
	}
}
