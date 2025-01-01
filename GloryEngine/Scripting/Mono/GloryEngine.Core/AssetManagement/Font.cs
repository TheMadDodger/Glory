using System;

namespace GloryEngine
{
    /// <summary>
    /// Font resource
    /// </summary>
    public class Font : Resource
    {
        #region Constructor

        public Font() : base() { }
        public Font(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
