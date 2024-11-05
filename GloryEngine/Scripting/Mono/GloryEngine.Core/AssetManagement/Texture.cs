using System;

namespace GloryEngine
{
    /// <summary>
    /// Texture handle
    /// </summary>
    public class Texture : Resource
    {
        #region Constructor

        public Texture() : base() { }
        public Texture(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
