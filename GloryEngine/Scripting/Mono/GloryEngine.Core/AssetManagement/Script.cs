using System;

namespace GloryEngine
{
    /// <summary>
    /// Script handle
    /// </summary>
    public class Script : Resource
    {
        #region Constructor

        public Script() : base() { }
        public Script(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
