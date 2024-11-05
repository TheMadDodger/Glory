using System;

namespace GloryEngine
{
    /// <summary>
    /// Mesh handle
    /// </summary>
    public class Mesh : Resource
    {
        #region Constructor

        public Mesh() : base() { }
        public Mesh(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
