using System;

namespace GloryEngine
{
    /// <summary>
    /// Prefab handle
    /// </summary>
    public class Prefab : Resource
    {
        #region Constructor

        public Prefab() : base() { }
        public Prefab(UInt64 id) : base(id) { }

        #endregion
    }
}
