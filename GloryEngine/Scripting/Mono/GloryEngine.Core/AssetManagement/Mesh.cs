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
        public Mesh(UInt64 id) : base(id) { }

        #endregion
    }
}
