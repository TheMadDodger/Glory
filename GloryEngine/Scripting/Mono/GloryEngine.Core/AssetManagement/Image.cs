using System;

namespace GloryEngine
{
    /// <summary>
    /// Image resource
    /// </summary>
    public class Image : Resource
    {
        #region Constructor

        public Image() : base() { }
        public Image(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
