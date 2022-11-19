using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GloryEngine
{
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
