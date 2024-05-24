using System;

namespace GloryEngine
{
    /// <summary>
    /// A handle for a camera
    /// </summary>
    public struct Camera
    {
        #region Props

        /// <summary>
        /// ID of the camera
        /// </summary>
        public UInt64 CameraID => _cameraID;

        #endregion

        #region Fields

        private UInt64 _cameraID;

        #endregion

        #region Constructor

        /// <summary>
        /// Construct a camera handle
        /// </summary>
        /// <param name="id">ID of the camera</param>
        public Camera(UInt64 id)
        {
            _cameraID = id;
        }

		#endregion
	}
}
