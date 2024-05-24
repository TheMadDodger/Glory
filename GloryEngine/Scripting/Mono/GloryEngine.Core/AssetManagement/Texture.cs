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
        public Texture(UInt64 id) : base(id) { }

        #endregion
    }
}
